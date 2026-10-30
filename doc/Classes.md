# Diagramme de classes du TP1

Ces diagrammes Mermaid décrivent les classes et les relations de la version actuelle du code dans `include/moteur/` et `src/moteur/`. Les vues sont séparées pour rester lisibles dans GitHub.

## Moteur, paramètres et ressources

```mermaid
classDiagram
    class Engine {
        +getInstance()
        +initialize()
        +applySettings()
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

    Engine *-- Window : possède
    Engine *-- Settings : possède
    Engine *-- ResourceManager : possède
    ResourceManager o-- Resource : partage par titre
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

    SceneManager *-- IScene : possède les scènes
    IScene <|-- MainScene
    IScene <|-- SettingsScene
    IScene <|-- CreditsScene
    IScene <|-- DemoGameScene
    IScene *-- EventManager : un par scène
    EventManager ..> EventObserver : notifie
    EventManager ..> Event : transmet
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

    Image o-- Texture : ressource partagée
    Label o-- Font : ressource partagée
    Button *-- Label
    Selector *-- Label
    Selector *-- Button : deux boutons de navigation
    Button --> Program
    Image --> Program
    Label --> Program
    ProgressBar --> Program
    Program ..> Shader : compile et lie
    VAO --> VBO : capture les attributs
    VBO ..|> Buffer
    Matrix4 ..> Program : fournit la matrice
```

## Relations et patrons

- `Engine` est un Singleton. Il possède la fenêtre, les paramètres et le gestionnaire de ressources.
- `SceneManager` possède les scènes par `unique_ptr`. Chaque scène possède son propre `EventManager`.
- `EventManager` applique le patron Observateur : il notifie les observateurs inscrits pour un type d’événement.
- `ResourceManager` conserve les ressources par titre avec des références partagées.
- Les scènes sont chargées par `SceneManager` au moment de leur activation, puis déchargées à leur désactivation.
- Les sept contrôles implémentent `VisualComponent`. `Button`, `CheckBox`, `Selector` et `Slider` implémentent aussi `EventObserver`.
- `Selector` compose une étiquette et deux boutons de navigation. Seul le sélecteur est inscrit au gestionnaire d’événements de sa scène.
- `VBO` réalise le contrat `Buffer`. `VAO` mémorise les attributs associés au tampon de sommets.

## Fichiers de diagrammes

- [Diagramme vectoriel SVG](Classes.svg) — rendu historique; il ne montre pas encore `VisualComponent` et `Buffer`.
- [Aperçu PNG](Classes.png) — rendu historique correspondant au SVG.
- [Diagramme UMLet initial](Classes-initiale.uxf) — document conservé comme historique de conception.

Les diagrammes Mermaid ci-dessus représentent les abstractions présentes dans le code actuel. Les rendus PNG et SVG peuvent être conservés comme archives ou supprimés plus tard si vous décidez de ne maintenir que le diagramme Mermaid.
