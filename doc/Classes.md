# Diagramme de classes du TP1

Ces diagrammes Mermaid décrivent la version actuelle du code dans `include/moteur/` et `src/moteur/`. **C’est le diagramme de classes courant à remettre.** Le fichier [`Classes-initiale.uxf`](Classes-initiale.uxf) est le modèle antérieur du début du projet; il ne décrit pas l’architecture actuelle. Les vues sont séparées pour rester lisibles dans GitHub.

## Moteur, paramètres et ressources

```mermaid
classDiagram
    class Engine {
        +getInstance()
        +initialize()
        +applySettings()
        +commitSettings(values)
        +run()
        +stop()
        +shutdown()
        -Window window
        -ResourceManager resources
        -Settings settings
    }
    class Window {
        +initialize()
        +applyDisplaySettings()
        +getPixelSize()
        +swapBuffers()
        +close()
    }
    class Settings {
        -filePath
        -language
        -width
        -height
        -fullscreen
        -vsync
        -sensitivity
        -fov
        -programBinaryFormat
        +load()
        +save()
        +getValues()
        +setValues(values)
    }
    class ResourceManager {
        -resources: map~string, shared_ptr Resource~
        +add()
        +get~T~()
        +remove()
        +clear()
    }
    class Resource {
        <<abstract>>
        #title
        #loaded
        #error
        +load(path)*
        +release()*
    }
    class Texture {
        -id
        -width
        -height
        +load(path)
        +setFiltering()
        +setWrapping()
        +release()
    }
    class Font {
        -handle
        -pointSize
        +load(path)
        +measureText()
        +createTextTexture()
        +release()
    }

    Engine "1" *-- "1" Window : membre par valeur
    Engine "1" *-- "1" Settings : membre par valeur
    Engine "1" *-- "1" ResourceManager : membre par valeur
    ResourceManager "1" o-- "0..*" Resource : map de shared_ptr, par titre
    Resource <|-- Texture
    Resource <|-- Font
```

## Scènes et événements

```mermaid
classDiagram
    class SceneManager {
        -scenes: map~string, unique_ptr IScene~
        -activeName
        +addScene()
        +removeScene()
        +setActiveScene()
        +dispatchEvent()
        +update()
        +render()
        +clear()
    }
    class IScene {
        <<abstract>>
        +load()*
        +unload()*
        +update()
        +render()
        +dispatchEvent()
    }
    class MainScene {
        +load()
        +unload()
        +render()
    }
    class SettingsScene {
        +load()
        +unload()
        +update()
        +render()
        -applyChanges()
    }
    class CreditsScene {
        +load()
        +unload()
        +render()
    }
    class DemoGameScene {
        +load()
        +unload()
        +update()
        +render()
    }
    class EventManager {
        -registrations: weak observers and event types
        +addObserver()
        +removeObserver()
        +notifyObservers()
    }
    class EventObserver {
        <<abstract>>
        +onEvent(event)*
    }
    class Event {
        -sdlEvent
        +getType()
        +getSDLEvent()
    }
    class Image
    class Label
    class Button
    class Selector
    class CheckBox
    class Slider
    class ProgressBar

    SceneManager "1" *-- "0..*" IScene : map de unique_ptr, par nom
    IScene "1" *-- "1" EventManager : membre par valeur
    IScene <|-- MainScene
    IScene <|-- SettingsScene
    IScene <|-- CreditsScene
    IScene <|-- DemoGameScene
    EventManager "1" ..> "0..*" EventObserver : observe par weak_ptr
    EventManager ..> Event : transmet
    MainScene "1" *-- "1" Image : composant possede
    MainScene "1" *-- "2" Label : composants possedes
    MainScene "1" *-- "4" Button : composants possedes
    SettingsScene "1" *-- "1" Image : composant possede
    SettingsScene "1" *-- "8" Label : composants possedes
    SettingsScene "1" *-- "2" Selector : composants possedes
    SettingsScene "1" *-- "2" CheckBox : composants possedes
    SettingsScene "1" *-- "2" Slider : composants possedes
    SettingsScene "1" *-- "2" Button : composants possedes
    CreditsScene "1" *-- "6" Label : composants possedes
    CreditsScene "1" *-- "1" Button : composant possede
    DemoGameScene "1" *-- "2" Label : composants possedes
    DemoGameScene "1" *-- "1" ProgressBar : composant possede
    DemoGameScene "1" *-- "1" Button : composant possede
```

## Rendu et contrôles visuels

```mermaid
classDiagram
    class VisualComponent {
        <<abstract>>
        +render()*
        +release()*
    }
    class Image {
        -shared_ptr Texture texture
        -position
        -dimensions
        +setTexture()
        +resize()
        +render()
        +release()
    }
    class Label {
        -shared_ptr Font font
        -text
        -position
        -color
        -texture
        +setFont()
        +setText()
        +getDimensions()
        +render()
        +release()
    }
    class Button {
        -Label label
        -onClick
        -hovered
        -pressed
        +onEvent()
        +setOnClick()
        +render()
        +release()
    }
    class CheckBox {
        -checked
        -position
        -size
        +onEvent()
        +isChecked()
        +render()
        +release()
    }
    class Selector {
        -items
        -selected
        -Label label
        -Button previousButton
        -Button nextButton
        +onEvent()
        +previous()
        +next()
        +render()
        +release()
    }
    class Slider {
        -minimum
        -value
        -maximum
        -increment
        -dragging
        +onEvent()
        +setValue()
        +render()
        +release()
    }
    class ProgressBar {
        -minimum
        -value
        -maximum
        +setRange()
        +setValue()
        +getProgress()
        +render()
        +release()
    }
    class EventObserver {
        <<abstract>>
        +onEvent(event)*
    }
    class Program {
        +link()
        +use()
        +getBinary()
        +loadBinary()
        +release()
    }
    class Shader {
        +compile()
        +loadShader()
        +release()
    }
    class Buffer {
        <<interface>>
        +create()*
        +setData()*
        +release()*
    }
    class VBO {
        -id
        +create()
        +setData()
        +release()
    }
    class VAO {
        -id
        +create()
        +setAttribute()
        +release()
    }
    class Matrix4 {
        -values: float[16]
        +orthographic()
        +operator*()
        +transformPoint()
        +data()
    }

    VisualComponent <|.. Image
    VisualComponent <|.. Label
    VisualComponent <|.. Button
    VisualComponent <|.. CheckBox
    VisualComponent <|.. Selector
    VisualComponent <|.. Slider
    VisualComponent <|.. ProgressBar

    EventObserver <|-- Button
    EventObserver <|-- CheckBox
    EventObserver <|-- Selector
    EventObserver <|-- Slider

    Image "0..*" --> "1" Texture : référence partagée
    Label "0..*" --> "1" Font : référence partagée
    Button "1" *-- "1" Label : membre par valeur
    Selector "1" *-- "1" Label : membre par valeur
    Selector "1" *-- "2" Button : navigation, membres par valeur
    Button "1" *-- "1" Program : membre par valeur
    Image "1" *-- "1" Program : membre par valeur
    Label "1" *-- "1" Program : membre par valeur
    ProgressBar "1" *-- "1" Program : membre par valeur
    Program ..> Shader : compile et lie
    VAO ..> VBO : capture les attributs liés
    VBO ..|> Buffer
    Matrix4 ..> Program : fournit la matrice
```

## Relations et patrons

- `Engine` est un Singleton. Il possède la fenêtre, les paramètres et le gestionnaire de ressources.
- `SceneManager` possède de zéro à plusieurs scènes par `unique_ptr`; au plus une scène est active. Il charge la destination avant de décharger la scène courante. Si le chargement échoue, il décharge la destination partielle et conserve l’ancienne scène active.
- Chaque `IScene` possède un `EventManager` par valeur. Ce gestionnaire observe de zéro à plusieurs composants via des `weak_ptr`; la scène garde la propriété de ses composants. `unload()` retire leurs inscriptions, y compris après un chargement partiel.
- `ResourceManager` détient des `shared_ptr<Resource>` pour ses entrées titrées. Les scènes et contrôles gardent aussi des références partagées aux ressources qu’ils utilisent. Aucun composant de scène ne doit appeler `Texture::release()` ou `Font::release()`; il libère ses propres composants, et la dernière référence détruite libère la ressource.
- `MainScene` possède une image, deux étiquettes et quatre boutons. `SettingsScene` possède une image, huit étiquettes et huit contrôles interactifs. `CreditsScene` possède six étiquettes et un bouton. `DemoGameScene` possède deux étiquettes, une barre de progression et un bouton. Les scènes libèrent leurs composants au déchargement.
- Les sept contrôles réalisent `VisualComponent`. `Button`, `CheckBox`, `Selector` et `Slider` réalisent aussi `EventObserver`. `Selector` possède une étiquette et deux boutons de navigation par valeur; seul le sélecteur est observé par les événements de la scène.
- Les programmes, VAO et VBO appartiennent aux contrôles par valeur. Le VAO capture la configuration du tampon lié, mais ne possède pas le VBO. `VBO` réalise le contrat `Buffer`.
- `SettingsScene` recueille les choix de l’interface et appelle `Engine::commitSettings`. `Engine` applique l’affichage à `Window`; `Settings` valide et sauvegarde les valeurs.


