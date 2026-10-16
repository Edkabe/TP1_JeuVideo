"""Génère Classes.uxf et Classes.png depuis le même modèle (Pillow requis)."""
from pathlib import Path
import xml.etree.ElementTree as ET
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent
FONT_DIR = Path('C:/Windows/Fonts')
FONT = ImageFont.truetype(str(FONT_DIR / 'arial.ttf'), 20)
BOLD = ImageFont.truetype(str(FONT_DIR / 'arialbd.ttf'), 23)
TITLE = ImageFont.truetype(str(FONT_DIR / 'arialbd.ttf'), 31)
SMALL = ImageFont.truetype(str(FONT_DIR / 'arial.ttf'), 18)
image = Image.new('RGB', (3280, 2920), '#ffffff')
draw = ImageDraw.Draw(image)
xml = ET.Element('diagram', program='umletino', version='15.1')
ET.SubElement(xml, 'zoom_level').text = '10'
ET.SubElement(xml, 'help_text').text = 'Scene reste abstraite. Les ressources polymorphes sont partagées par shared_ptr. Modèle généré par generer_diagramme.py.'
nodes = {}

def element(kind, x, y, w, h, text, extra=''):
    el = ET.SubElement(xml, 'element')
    ET.SubElement(el, 'id').text = kind
    c = ET.SubElement(el, 'coordinates')
    for key, value in zip(('x', 'y', 'w', 'h'), (x, y, w, h)):
        ET.SubElement(c, key).text = str(value)
    ET.SubElement(el, 'panel_attributes').text = text
    ET.SubElement(el, 'additional_attributes').text = extra

def panel(x, y, title, note):
    draw.rectangle((x, y, x+1600, y+1410), fill='#f8fafc', outline='#6b8094', width=2)
    draw.text((x+24, y+18), title, font=TITLE, fill='#172b40')
    draw.text((x+24, y+1360), note, font=SMALL, fill='#44566a')
    element('UMLNote', x+20, y+15, 1500, 60, title)

def box(key, x, y, name, attrs, methods):
    w = 480
    h = 50 + 27*(len(attrs)+len(methods)) + (14 if attrs else 0)
    draw.rectangle((x, y, x+w, y+h), fill='white', outline='#73879a', width=2)
    draw.rectangle((x, y, x+w, y+42), fill='#e0edf3', outline='#73879a', width=2)
    draw.text((x+12, y+7), name, font=BOLD, fill='#172b40')
    ty = y+49
    for line in attrs:
        draw.text((x+12, ty), line, font=FONT, fill='#253447'); ty += 27
    if attrs:
        draw.line((x, ty+2, x+w, ty+2), fill='#b1bdc8'); ty += 14
    for line in methods:
        draw.text((x+12, ty), line, font=FONT, fill='#253447'); ty += 27
    nodes[key] = (x, y, w, h)
    element('UMLClass', x, y, w, h, '\n'.join([name, '--', *attrs, '--', *methods]))

def relation(a, b, label, inheritance=False):
    ax, ay, aw, ah = nodes[a]; bx, by, bw, bh = nodes[b]
    if by >= ay+ah:
        start, end = (ax+aw//2, ay+ah), (bx+bw//2, by)
    elif bx >= ax+aw:
        start, end = (ax+aw, ay+ah//2), (bx, by+bh//2)
    else:
        start, end = (ax, ay+ah//2), (bx+bw, by+bh//2)
    # Les héritages sont dirigés vers la classe de base.
    if inheritance:
        start, end = end, start
    draw.line((start, end), fill='#6b8094', width=2)
    if inheritance:
        import math
        dx, dy = end[0]-start[0], end[1]-start[1]
        length = math.hypot(dx,dy); ux,uy=dx/length,dy/length
        p1=(end[0]-18*ux+8*uy,end[1]-18*uy-8*ux)
        p2=(end[0]-18*ux-8*uy,end[1]-18*uy+8*ux)
        draw.polygon((end,p1,p2), fill='white', outline='#6b8094', width=2)
    mx,my=(start[0]+end[0])//2,(start[1]+end[1])//2
    if label:
        # Dans les petits espaces horizontaux, placer la légende au-dessus
        # des boîtes plutôt que sur leur contenu.
        if abs(ay-by) < 60:
            my = min(ay, by)-28
        mx -= int(draw.textlength(label,font=SMALL))//2
        bounds=draw.textbbox((mx,my),label,font=SMALL)
        draw.rectangle((bounds[0]-3,bounds[1]-2,bounds[2]+3,bounds[3]+2),fill='#f8fafc')
        draw.text((mx,my),label,font=SMALL,fill='#44566a')
    x,y=min(start[0],end[0]),min(start[1],end[1])
    element('Relation',x,y,max(abs(end[0]-start[0]),10),max(abs(end[1]-start[1]),10),
            ('lt=<<-' if inheritance else 'lt=-')+'\n'+label,
            f'{start[0]-x};{start[1]-y};{end[0]-x};{end[1]-y}')

panel(20,20,'1. Moteur et paramètres','Engine possède une fenêtre unique et ses gestionnaires. Les accès retournent des références.')
box('engine',580,110,'Engine', ['- static instance : Engine','- running : bool'], ['+ static getInstance() : Engine&','+ run() : bool','+ stop() : void','+ getSettings() : EngineSettings&'])
box('window',60,460,'Window',['- window : SDL_Window*','- context : SDL_GLContext'],['+ initialize() : bool','+ swapBuffers() : void','+ close() : void'])
box('settings',580,460,'EngineSettings',['- language : string','- resolution : Vector2','- fullscreen, vsync : bool','- mouseSensitivity, fov : float','- programBinaryFormat : GLenum'],['+ load() : bool','+ save() : bool','+ getters / setters'])
box('sm1',1100,460,'SceneManager',['- current : Scene*'],['+ add(name, scene) : bool','+ remove(name) : bool','+ change(name) : bool'])
box('rm1',60,960,'ResourceManager',[],['+ add<T>(name, resource) : bool','+ get<T>(name) : shared_ptr<T>','+ remove(name) : bool'])
box('renderer1',1100,960,'Renderer2D',['- program : Program','- viewProjection : Matrix4'],['+ begin() : void','+ draw(component) : void','+ end() : void'])
for child,label in [('window','1 fenêtre'),('settings','1 configuration'),('sm1','1 gestionnaire'),('rm1','1 gestionnaire'),('renderer1','1 rendu')]:
    # Les associations inférieures sont indiquées dans la note pour éviter des lignes traversant les classes.
    if child in ('window','settings','sm1'): relation('engine',child,label)

panel(1660,20,'2. Ressources et rendu','Resource est abstraite : stockage et récupération par pointeurs partagés, sans copie de la ressource.')
box('rm',1700,110,'ResourceManager',['- resources : unordered_map<','    string, shared_ptr<Resource>>'],['+ add<T>(name, resource) : bool','+ get<T>(name) : shared_ptr<T>','+ remove(name) : bool','+ clear() : void'])
box('resource',2220,110,'«abstract» Resource',[],['+ virtual ~Resource()','+ virtual load() : bool = 0','+ virtual release() : void = 0'])
box('texture',2220,470,'Texture',['- id : GLuint','- width, height : int'],['+ load() : bool','+ release() : void','+ bind() : void','+ explicit operator GLuint() const'])
box('font',2740,470,'Font',['- handle : TTF_Font*','- pointSize : float'],['+ load() : bool','+ release() : void','+ explicit operator TTF_Font*() const'])
box('program',1700,940,'Program',['- id : GLuint'],['+ attach(shader) : void','+ link() : bool','+ use() : void','+ loadBinary(path) : bool','+ saveBinary(path) : bool'])
box('shader',2220,940,'Shader',['- id : GLuint','- type : ShaderType'],['+ compile(path) : bool','+ release() : void'])
box('buffer',2740,940,'Buffer',['- id : GLuint'],['+ upload(data) : void','+ bind() : void','+ release() : void'])
relation('rm','resource','0..* ressources')
relation('resource','texture','',True); relation('resource','font','',True)
relation('program','shader','lie 1..*')

panel(20,1470,'3. Scènes et événements','Chaque Scene possède son EventManager. Les transitions activent/désactivent une seule scène à la fois.')
box('sm',60,1560,'SceneManager',['- scenes : unordered_map<','    string, unique_ptr<Scene>>','- current : Scene*'],['+ add(name, scene) : bool','+ remove(name) : bool','+ change(name) : bool'])
box('scene',580,1560,'«abstract» Scene',['- events : EventManager','- controls : vector<','    unique_ptr<VisualComponent>>'],['+ virtual ~Scene()','+ virtual activate() : bool = 0','+ virtual deactivate() : void = 0','+ virtual handleEvents() : void = 0','+ virtual update(dt) : void = 0','+ virtual render() : void = 0'])
box('em',1100,1560,'EventManager',['- observers : EventObserver*[]'],['+ subscribe(observer) : void','+ unsubscribe(observer) : void','+ dispatch(event) : void'])
box('main',60,2120,'MainScene',[],['+ activate() : bool','+ deactivate() : void','+ handleEvents() : void','+ update(dt) : void','+ render() : void'])
box('params',580,2120,'SettingsScene',[],['+ activate() : bool','+ deactivate() : void','+ handleEvents() : void','+ update(dt) : void','+ render() : void'])
box('credits',1100,2120,'CreditsScene',[],['+ activate() : bool','+ deactivate() : void','+ handleEvents() : void','+ update(dt) : void','+ render() : void'])
box('observer',60,2550,'«abstract» EventObserver',[],['+ virtual ~EventObserver()','+ virtual onEvent(event) : void = 0'])
box('event',1100,2550,'Event',['- type : EventType','- payload : données'],[])
relation('sm','scene','0..* scènes'); relation('scene','em','1 par scène')
for child in ('main','params','credits'): relation('scene',child,'',True)

panel(1660,1470,'4. Contrôles visuels','Les 7 contrôles dérivent de VisualComponent. Button, CheckBox et Slider observent les événements.')
box('visual',2220,1560,'«abstract» VisualComponent',['# position, size : Vector2','- visible : bool'],['+ virtual ~VisualComponent()','+ update(dt) : void','+ virtual render(renderer) : void = 0'])
box('image',1700,1950,'Image',['- texture : shared_ptr<Texture>'],['+ resize(w, h, keepAspect) : void'])
box('label',2220,1950,'Label',['- font : shared_ptr<Font>','- text : string','- color : Color'],['+ setText(text) : void'])
box('button',2740,1950,'Button',['- title : Label','- border, background, hover : Color'],['+ onEvent(event) : void','+ événement clicked'])
box('progress',1700,2300,'ProgressBar',['- min, value, max : float','- border, background, fill : Color'],['+ setValue(value) : void'])
box('check',2220,2300,'CheckBox',['- checked : bool','- border, background, tick : Color'],['+ isChecked() : bool','+ onEvent(event) : void'])
box('selector',2740,2300,'Selector',['- items : vector<string>','- selectedIndex : int','- previous, next : Button','- selectedLabel : Label'],['+ getSelected() : string'])
box('slider',1700,2620,'Slider',['- min, value, max, step : float','- border, background, fill : Color','- hover : Color'],['+ getValue() : float','+ onEvent(event) : void'])
box('vector',2220,2620,'Vector2 / Matrix4',[],['types et opérations basés sur GLM'])
box('color',2740,2620,'Color',['+ r, g, b, a : float'],[])
for child in ('image','label','button'): relation('visual',child,'',True)
# Les héritages des quatre contrôles inférieurs figurent aussi dans le source UML.
for child in ('progress','check','selector','slider'):
    ax,ay,aw,ah=nodes['visual']; bx,by,bw,bh=nodes[child]
    element('Relation', ax,ay, max(abs(bx-ax),10), by-ay,
            'lt=<<-\n', f'240;{ah};{bx-ax+240};{by-ay}')

image.save(ROOT/'Classes.png')
ET.indent(xml, space='  ')
ET.ElementTree(xml).write(ROOT/'Classes.uxf', encoding='utf-8', xml_declaration=True)
print('Classes.png et Classes.uxf générés.')
