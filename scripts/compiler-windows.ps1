param([switch]$Verifier, [switch]$TesterRessources, [switch]$TesterMathematiques)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
Set-Location -LiteralPath $projectRoot

# Le même compilateur MSVC x64 que dans l'environnement Windows du cours.
if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
    if (-not (Test-Path -LiteralPath $vswhere)) { throw 'Installer les Build Tools Visual Studio avec la charge C++.' }
    $vsPath = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (-not $vsPath) { throw 'Compilateur MSVC C++ introuvable.' }
    & (Join-Path $vsPath 'Common7/Tools/Launch-VsDevShell.ps1') -Arch amd64 -HostArch amd64 -SkipAutomaticLocation
}
if ($env:VSCMD_ARG_TGT_ARCH -ne 'x64') {
    throw 'Utiliser un terminal développeur MSVC x64.'
}
$output = Join-Path $projectRoot 'build'
$objects = Join-Path $output 'obj'
New-Item -ItemType Directory -Path $objects -Force | Out-Null
$options = @('/nologo','/std:c++17','/EHsc','/Z7','/MT','/W4','/utf-8',
    "/I$projectRoot/include", "/I$projectRoot/build/include", "/I$projectRoot/external/glm")
$compiler = (Get-Command cl.exe).Source
$signature = $compiler + '|' + (Get-Item -LiteralPath $compiler).LastWriteTimeUtc.Ticks + '|' + ($options -join '|')

function Compile-Source([string]$Source, [string]$Object) {
    $dependencies = $Object + '.json'
    $signaturePath = $Object + '.options'
    $rebuild = -not (Test-Path -LiteralPath $Object) -or
        -not (Test-Path -LiteralPath $dependencies) -or
        -not (Test-Path -LiteralPath $signaturePath)
    if (-not $rebuild) {
        $rebuild = (Get-Content -LiteralPath $signaturePath -Raw) -ne $signature
        try {
            $data = Get-Content -LiteralPath $dependencies -Raw | ConvertFrom-Json
            $inputs = @($Source) + @($data.Data.Includes)
            $objectTime = (Get-Item -LiteralPath $Object).LastWriteTimeUtc
            foreach ($inputPath in $inputs) {
                if (-not (Test-Path -LiteralPath $inputPath) -or
                    (Get-Item -LiteralPath $inputPath).LastWriteTimeUtc -gt $objectTime) { $rebuild = $true; break }
            }
        } catch { $rebuild = $true }
    }
    if ($rebuild) {
        Write-Host "Compilation : $([IO.Path]::GetFileName($Source))"
        & cl.exe @options /c $Source "/Fo$Object" /sourceDependencies $dependencies | Out-Host
        if ($LASTEXITCODE -ne 0) { throw "Compilation échouée : $Source" }
        [IO.File]::WriteAllText($signaturePath, $signature)
    }
    return $rebuild
}

$sources = @(Get-ChildItem -LiteralPath (Join-Path $projectRoot 'src/moteur') -Filter '*.cpp' -Recurse | Sort-Object FullName)
if (-not $sources.Count) { throw 'Aucune source moteur.' }
$engineObjects = @()
$engineChanged = $false
foreach ($source in $sources) {
    $relative = $source.FullName.Substring((Join-Path $projectRoot 'src/moteur').Length + 1)
    $object = Join-Path $objects ('moteur/' + [IO.Path]::ChangeExtension($relative, '.obj'))
    New-Item -ItemType Directory -Path (Split-Path -Parent $object) -Force | Out-Null
    # La sortie du compilateur est envoyée à l'hôte, pas à la valeur retournée.
    if (Compile-Source $source.FullName $object) { $engineChanged = $true }
    $engineObjects += $object
}
$library = Join-Path $output 'lib/moteur.lib'
$libraryInputs = Join-Path $objects 'moteur.inputs'
$sourceSignature = $engineObjects -join '|'
$archiveChanged = $engineChanged -or -not (Test-Path -LiteralPath $library) -or
    -not (Test-Path -LiteralPath $libraryInputs)
if (-not $archiveChanged) { $archiveChanged = (Get-Content -LiteralPath $libraryInputs -Raw) -ne $sourceSignature }
if ($archiveChanged) {
    Write-Host 'Archivage : moteur.lib'
    & lib.exe /nologo "/OUT:$library" @engineObjects
    if ($LASTEXITCODE -ne 0) { throw 'Création de moteur.lib échouée.' }
    [IO.File]::WriteAllText($libraryInputs, $sourceSignature)
}
$mainObject = Join-Path $objects 'main.obj'
$mainChanged = Compile-Source (Join-Path $projectRoot 'src/main.cpp') $mainObject
$executable = Join-Path $output 'program.exe'
$linkInputs = @($mainObject, $library, (Join-Path $output 'lib/gl.lib'),
    (Join-Path $output 'lib/SDL3.lib'), (Join-Path $output 'lib/SDL3_image.lib'), (Join-Path $output 'lib/SDL3_ttf.lib'))
$linkChanged = $mainChanged -or $archiveChanged -or -not (Test-Path -LiteralPath $executable)
if (-not $linkChanged) {
    foreach ($inputPath in $linkInputs) {
        if ((Get-Item -LiteralPath $inputPath).LastWriteTimeUtc -gt (Get-Item -LiteralPath $executable).LastWriteTimeUtc) { $linkChanged = $true; break }
    }
}
if ($linkChanged) {
    Write-Host 'Liaison : program.exe'
    & link.exe /nologo /debug /machine:x64 /subsystem:windows "/OUT:$executable" @linkInputs opengl32.lib
    if ($LASTEXITCODE -ne 0) { throw 'Liaison échouée.' }
} else { Write-Host 'Compilation à jour : aucun objet ni exécutable reconstruit.' }
$shaderOutput = Join-Path $output 'assets/shaders'
New-Item -ItemType Directory -Path $shaderOutput -Force | Out-Null
Get-ChildItem -LiteralPath (Join-Path $projectRoot 'assets/shaders') -File | ForEach-Object {
    Copy-Item -LiteralPath $_.FullName -Destination (Join-Path $shaderOutput $_.Name) -Force
}
if ($Verifier) {
    & $executable --check
    if ($LASTEXITCODE -ne 0) { throw 'Test des dépendances échoué.' }
}
if ($TesterRessources) {
    $testObject = Join-Path $objects 'ResourceTests.obj'
    $testChanged = Compile-Source (Join-Path $projectRoot 'tests/ResourceTests.cpp') $testObject
    $testExecutable = Join-Path $output 'resource-tests.exe'
    if ($testChanged -or $archiveChanged -or $linkChanged -or -not (Test-Path -LiteralPath $testExecutable)) {
        & link.exe /nologo /debug /machine:x64 /subsystem:console "/OUT:$testExecutable" $testObject $library "/LIBPATH:$output/lib" gl.lib SDL3.lib SDL3_image.lib SDL3_ttf.lib opengl32.lib
        if ($LASTEXITCODE -ne 0) { throw 'Liaison des tests de ressources echouee.' }
    }
    $testAssets = Join-Path $output 'assets/tests'
    New-Item -ItemType Directory -Path $testAssets -Force | Out-Null
    Copy-Item -LiteralPath (Join-Path $projectRoot 'tests/assets/pixels.png') -Destination $testAssets -Force
    & $testExecutable
    if ($LASTEXITCODE -ne 0) { throw 'Tests de ressources echoues.' }
}

if ($TesterMathematiques) {
    $mathExecutable = Join-Path $output 'math-tests.exe'
    $mathSource = Join-Path $projectRoot 'tests/MathTests.cpp'
    Write-Host 'Compilation : MathTests.cpp'
    & cl.exe @options $mathSource "/Fe$mathExecutable" "/Fo$objects/MathTests.obj" /link /nologo
    if ($LASTEXITCODE -ne 0) { throw 'Compilation des tests mathématiques échouée.' }
    & $mathExecutable
    if ($LASTEXITCODE -ne 0) { throw 'Tests mathématiques échoués.' }
}
