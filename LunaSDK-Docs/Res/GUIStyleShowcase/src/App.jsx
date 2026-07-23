import { useEffect, useMemo, useState } from "react";
import {
  ArrowLeft,
  ArrowRight,
  ArrowUp,
  ArrowsClockwise,
  ArrowsOut,
  Bell,
  BoundingBox,
  Camera,
  CaretDown,
  CaretRight,
  ChartLineUp,
  Check,
  Code,
  Columns,
  Command,
  Copy,
  Cube,
  CursorClick,
  DotsSixVertical,
  DotsThree,
  Eye,
  File,
  FloppyDisk,
  Folder,
  Funnel,
  GearSix,
  GridFour,
  HandTap,
  ImageSquare,
  Info,
  Lightning,
  LinkSimple,
  ListBullets,
  MagnifyingGlass,
  Moon,
  Mouse,
  PaintBrush,
  Palette,
  Play,
  Plus,
  Selection,
  Sidebar,
  SlidersHorizontal,
  SquaresFour,
  Stack,
  Sun,
  Terminal,
  Timer,
  Trash,
  TreeStructure,
  UploadSimple,
  VectorThree,
  Warning,
  Wrench,
  X,
} from "@phosphor-icons/react";
import { BuiltInIconCatalog } from "./iconCatalog.jsx";

const sections = [
  { id: "overview", label: "Overview", icon: SquaresFour },
  { id: "primitives", label: "Primitives", icon: Stack },
  { id: "buttons", label: "Buttons", icon: CursorClick },
  { id: "input", label: "Input", icon: SlidersHorizontal },
  { id: "layouts", label: "Layouts", icon: GridFour },
  { id: "scroll", label: "Scroll Views", icon: Columns },
  { id: "tables", label: "Tables", icon: ListBullets },
  { id: "overlay", label: "Overlay", icon: Bell },
  { id: "workspace", label: "Workspace", icon: Sidebar },
];

const materialAssets = [
  { name: "M_Desert_Sand", meta: "Material · 3.6 MB", image: "/material-sand.png" },
  { name: "M_Rusted_Metal", meta: "Material · 2.1 MB", image: "/material-rusted.png" },
  { name: "M_Concrete_Grime", meta: "Material · 1.8 MB", image: "/material-concrete.png" },
];

function contrastInk(hexColor) {
  const hex = hexColor.replace("#", "");
  const channels = [0, 2, 4].map((offset) => parseInt(hex.slice(offset, offset + 2), 16) / 255);
  const [red, green, blue] = channels.map((channel) => (
    channel <= 0.04045 ? channel / 12.92 : ((channel + 0.055) / 1.055) ** 2.4
  ));
  const luminance = (0.2126 * red) + (0.7152 * green) + (0.0722 * blue);
  return luminance > 0.179 ? "#151718" : "#ffffff";
}

function hexChannels(hexColor) {
  const hex = hexColor.replace("#", "");
  return [0, 2, 4].map((offset) => parseInt(hex.slice(offset, offset + 2), 16));
}

function IconButton({ label, children, active = false, pressed, disabled = false, className = "", onClick }) {
  return (
    <button
      className={`icon-button ${active ? "is-active" : ""} ${className}`}
      aria-label={label}
      aria-pressed={pressed}
      title={label}
      disabled={disabled}
      onClick={onClick}
    >
      {children}
    </button>
  );
}

function Led({ tone = "off", shape = "dot", pulse = false }) {
  return <span className={`led led-${shape} led-${tone} ${pulse ? "is-pulsing" : ""}`} aria-hidden="true" />;
}

function StatusPill({ tone, children, count }) {
  const statusLabel = {
    success: "ready",
    busy: "working",
    warning: "warning",
    error: "error",
    off: "offline",
  }[tone] ?? "inactive";

  return (
    <span className="status-pill" aria-label={`${children}: ${statusLabel}`}>
      <Led tone={tone} />
      <span>{children}</span>
      {count !== undefined && <strong>{count}</strong>}
    </span>
  );
}

function Section({ id, eyebrow, title, description, children, wide = false }) {
  return (
    <section className={`showcase-section ${wide ? "is-wide" : ""}`} id={id}>
      <div className="section-heading">
        <div>
          <span className="eyebrow">{eyebrow}</span>
          <h2>{title}</h2>
        </div>
        <p>{description}</p>
      </div>
      {children}
    </section>
  );
}

function Card({ title, note, children, className = "" }) {
  return (
    <article className={`demo-card ${className}`}>
      {(title || note) && (
        <header className="demo-card-header">
          <div>
            <h3>{title}</h3>
            {note && <p>{note}</p>}
          </div>
        </header>
      )}
      <div className="demo-card-body">{children}</div>
    </article>
  );
}

function Segmented({ items, value, onChange, disabled = false, multiple = false }) {
  const selected = Array.isArray(value) ? value : [value];
  return (
    <div className="segmented" aria-label="Segmented selection">
      {items.map((item) => (
        <button
          key={item}
          className={selected.includes(item) ? "is-selected" : ""}
          aria-pressed={selected.includes(item)}
          disabled={disabled}
          onClick={() => {
            if (multiple) {
              onChange(selected.includes(item) ? selected.filter((valueItem) => valueItem !== item) : [...selected, item]);
            } else {
              onChange(item);
            }
          }}
        >
          {item}
        </button>
      ))}
    </div>
  );
}

function Toggle({ checked, onChange, label, disabled = false }) {
  return (
    <button
      type="button"
      className={`toggle ${checked ? "is-on" : ""}`}
      role="switch"
      aria-checked={checked}
      aria-label={label}
      disabled={disabled}
      onClick={() => onChange(!checked)}
    >
      <span />
    </button>
  );
}

function PropertyRow({ label, children }) {
  return (
    <div className="property-row" role="group" aria-label={label}>
      <span aria-hidden="true">{label}</span>
      <div>{children}</div>
    </div>
  );
}

function MiniEditor({ selectedAsset, setSelectedAsset, roughness, setRoughness, triplanar, setTriplanar }) {
  const [normalMap, setNormalMap] = useState(true);
  const [metallic, setMetallic] = useState(82);
  const [activeTool, setActiveTool] = useState("translate");
  const [spaceMode, setSpaceMode] = useState("local");
  const [consoleTab, setConsoleTab] = useState("console");
  const [inspectorTab, setInspectorTab] = useState("inspector");
  const selectedAssetData = materialAssets.find((asset) => asset.name === selectedAsset) ?? materialAssets[1];

  return (
    <div className="mini-editor" aria-label="DCC editor composition preview">
      <aside className="mini-library">
        <div className="mini-panel-title">
          <strong>Scene Hierarchy</strong>
          <IconButton label="Add object"><Plus size={17} /></IconButton>
        </div>
        <div className="search-field compact-search">
          <MagnifyingGlass size={16} />
          <input aria-label="Search scene" placeholder="Search scene…" />
          <Funnel size={16} />
        </div>
        <div className="tree-list">
          <div className="tree-row"><CaretDown size={14} /><Cube size={16} /><strong>Desert_Outpost</strong></div>
          <div className="tree-row depth-1"><CaretDown size={14} /><Folder size={16} />Environment</div>
          <div className="tree-row depth-2"><Sun size={16} />Sun_Light <Eye size={15} /></div>
          <div className="tree-row depth-1"><CaretDown size={14} /><Folder size={16} />Props</div>
          <div className="tree-row depth-2 is-selected"><Cube size={16} />Material_Ball <Eye size={15} /></div>
        </div>
        <div className="mini-assets-title">
          <strong>Assets</strong>
          <span>3 items</span>
        </div>
        <div className="asset-list">
          {materialAssets.map((asset) => (
            <button
              key={asset.name}
              className={`asset-row ${selectedAsset === asset.name ? "is-selected" : ""}`}
              onClick={() => setSelectedAsset(asset.name)}
            >
              <img src={asset.image} alt="" />
              <span><strong>{asset.name}</strong><small>{asset.meta}</small></span>
            </button>
          ))}
        </div>
      </aside>

      <div className="mini-viewport-column">
        <div className="viewport-toolbar">
          <div className="tool-group">
            <IconButton label="Select" active={activeTool === "select"} pressed={activeTool === "select"} onClick={() => setActiveTool("select")}><Selection size={19} /></IconButton>
            <IconButton label="Translate" active={activeTool === "translate"} pressed={activeTool === "translate"} onClick={() => setActiveTool("translate")}><ArrowsOut size={19} /></IconButton>
            <IconButton label="Rotate" active={activeTool === "rotate"} pressed={activeTool === "rotate"} onClick={() => setActiveTool("rotate")}><ArrowsClockwise size={19} /></IconButton>
            <IconButton label="Frame selection" onClick={() => setActiveTool("frame")} active={activeTool === "frame"} pressed={activeTool === "frame"}><BoundingBox size={19} /></IconButton>
          </div>
          <div className="segmented micro-segmented">
            <button className={spaceMode === "local" ? "is-selected" : ""} aria-pressed={spaceMode === "local"} onClick={() => setSpaceMode("local")}>Local</button>
            <button className={spaceMode === "world" ? "is-selected" : ""} aria-pressed={spaceMode === "world"} onClick={() => setSpaceMode("world")}>World</button>
          </div>
          <IconButton label="More viewport options"><DotsThree size={20} weight="bold" /></IconButton>
        </div>
        <div className="viewport-image">
          <img src="/material-preview.png" alt="Rusted metal material sphere in a desert scene" />
          <div className="viewport-hud">
            <span>Perspective</span>
            <span>60 FPS</span>
          </div>
          <div className="viewport-orbit-tools">
            <IconButton label="Camera"><Camera size={18} /></IconButton>
            <IconButton label="Fullscreen"><ArrowsOut size={18} /></IconButton>
          </div>
        </div>
        <div className="mini-console">
          <div className="tab-strip compact-tabs">
            <button className={consoleTab === "console" ? "is-active" : ""} aria-pressed={consoleTab === "console"} onClick={() => setConsoleTab("console")}>Console</button>
            <button className={consoleTab === "bake" ? "is-active" : ""} aria-pressed={consoleTab === "bake"} onClick={() => setConsoleTab("bake")}>Bake Queue <span className="count-badge">2</span></button>
            <button className={consoleTab === "messages" ? "is-active" : ""} aria-pressed={consoleTab === "messages"} onClick={() => setConsoleTab("messages")}>Messages</button>
          </div>
          <div className="console-line"><Led tone="success" /> <span>Renderer initialized successfully.</span><time>10:23:45</time></div>
          <div className="console-line"><Led tone="warning" /> <span>Bake queue contains two materials.</span><time>10:24:12</time></div>
        </div>
      </div>

      <aside className="mini-inspector">
        <div className="inspector-object">
          <img src={selectedAssetData.image} alt="" />
          <span><strong>{selectedAsset}</strong><small>Material</small></span>
          <IconButton label="Object menu"><CaretDown size={16} /></IconButton>
        </div>
        <div className="tab-strip compact-tabs">
          <button className={inspectorTab === "inspector" ? "is-active" : ""} aria-pressed={inspectorTab === "inspector"} onClick={() => setInspectorTab("inspector")}>Inspector</button>
          <button className={inspectorTab === "channels" ? "is-active" : ""} aria-pressed={inspectorTab === "channels"} onClick={() => setInspectorTab("channels")}>Channels</button>
        </div>
        <div className="inspector-form">
          <PropertyRow label="Shader Model">
            <select aria-label="Shader Model" defaultValue="PBR Metallic Roughness"><option>PBR Metallic Roughness</option><option>Unlit</option></select>
          </PropertyRow>
          <PropertyRow label="Base Color">
            <div className="color-control"><input type="color" aria-label="Base Color" defaultValue="#9a6038" /><code>#9A6038</code></div>
          </PropertyRow>
          <PropertyRow label="Roughness">
            <div className="inline-range"><input type="range" aria-label="Roughness" min="0" max="100" value={roughness} onInput={(event) => setRoughness(Number(event.currentTarget.value))} onChange={(event) => setRoughness(Number(event.currentTarget.value))} /><output>{(roughness / 100).toFixed(2)}</output></div>
          </PropertyRow>
          <PropertyRow label="Metallic">
            <div className="inline-range"><input type="range" aria-label="Metallic" min="0" max="100" value={metallic} onInput={(event) => setMetallic(Number(event.currentTarget.value))} onChange={(event) => setMetallic(Number(event.currentTarget.value))} /><output>{(metallic / 100).toFixed(2)}</output></div>
          </PropertyRow>
          <PropertyRow label="Normal Map">
            <Toggle label="Normal Map" checked={normalMap} onChange={setNormalMap} />
          </PropertyRow>
          <PropertyRow label="Tiling">
            <div className="vector-field"><input defaultValue="1.000" aria-label="Tiling X" /><input defaultValue="1.000" aria-label="Tiling Y" /></div>
          </PropertyRow>
          <PropertyRow label="Triplanar">
            <label className="check-row"><input type="checkbox" checked={triplanar} onChange={(event) => setTriplanar(event.target.checked)} /> Use Triplanar Mapping</label>
          </PropertyRow>
          <PropertyRow label="Detail Normal">
            <Toggle label="Detail Normal" checked={false} onChange={() => {}} disabled />
          </PropertyRow>
        </div>
      </aside>
    </div>
  );
}

export function App() {
  const [theme, setTheme] = useState("light");
  const [density, setDensity] = useState("touch");
  const [accent, setAccent] = useState("#e34f59");
  const [activeSection, setActiveSection] = useState("overview");
  const [selectedAsset, setSelectedAsset] = useState("M_Rusted_Metal");
  const [roughness, setRoughness] = useState(36);
  const [triplanar, setTriplanar] = useState(true);
  const [groupValue, setGroupValue] = useState("Run");
  const [multiValue, setMultiValue] = useState(["Build", "Profile"]);
  const [checkbox, setCheckbox] = useState(true);
  const [radio, setRadio] = useState("Radio B");
  const [toggle, setToggle] = useState(true);
  const [disclosure, setDisclosure] = useState(true);
  const [sliderValue, setSliderValue] = useState(42);
  const [subdivisions, setSubdivisions] = useState(64);
  const [previewColor, setPreviewColor] = useState("#d9535d");
  const [activeTab, setActiveTab] = useState("Material");
  const [menuOpen, setMenuOpen] = useState(false);
  const [popupOpen, setPopupOpen] = useState(false);
  const [tooltipOpen, setTooltipOpen] = useState(false);
  const [dialogOpen, setDialogOpen] = useState(false);
  const [tableSelection, setTableSelection] = useState(1);

  const styleName = useMemo(() => `${theme}.${density === "pointer" ? "compact" : "touch"}`, [theme, density]);
  const accentInk = useMemo(() => contrastInk(accent), [accent]);
  const previewChannels = useMemo(() => hexChannels(previewColor), [previewColor]);

  useEffect(() => {
    const closeTransientLayers = (event) => {
      if (event.key !== "Escape") return;
      setMenuOpen(false);
      setPopupOpen(false);
      setTooltipOpen(false);
      setDialogOpen(false);
    };
    window.addEventListener("keydown", closeTransientLayers);
    return () => window.removeEventListener("keydown", closeTransientLayers);
  }, []);

  const navigateTo = (id) => {
    setActiveSection(id);
    document.getElementById(id)?.scrollIntoView({ behavior: "smooth", block: "start" });
  };

  return (
    <div
      className="app-shell"
      data-theme={theme}
      data-density={density}
      style={{ "--accent": accent, "--accent-ink": accentInk }}
    >
      <header className="topbar">
        <div className="brand-lockup">
          <span className="brand-icon" aria-hidden="true"><Cube size={24} weight="duotone" /></span>
          <div><strong>Luna GUI</strong><small>Design Language Lab</small></div>
        </div>
        <div className="top-status" aria-label="System status">
          <StatusPill tone="success">Renderer Online</StatusPill>
          <StatusPill tone="busy" count={2}>Bake Queue</StatusPill>
          <StatusPill tone="off">Remote Sync</StatusPill>
          <StatusPill tone="error" count={1}>Error</StatusPill>
        </div>
        <div className="style-switcher">
          <div className="segmented top-segmented" aria-label="Color theme">
            <button className={theme === "light" ? "is-selected" : ""} aria-pressed={theme === "light"} onClick={() => setTheme("light")}><Sun size={17} />Light</button>
            <button className={theme === "dark" ? "is-selected" : ""} aria-pressed={theme === "dark"} onClick={() => setTheme("dark")}><Moon size={17} />Dark</button>
          </div>
          <div className="segmented top-segmented" aria-label="Input density">
            <button className={density === "pointer" ? "is-selected" : ""} aria-pressed={density === "pointer"} onClick={() => setDensity("pointer")}><Mouse size={17} />Compact</button>
            <button className={density === "touch" ? "is-selected" : ""} aria-pressed={density === "touch"} onClick={() => setDensity("touch")}><HandTap size={17} />Touch</button>
          </div>
          <label className="accent-picker" title="Accent color">
            <input
              type="color"
              value={accent}
              onInput={(event) => setAccent(event.currentTarget.value)}
              onChange={(event) => setAccent(event.currentTarget.value)}
              aria-label="Accent color"
            />
            <span>{accent.toUpperCase()}</span>
          </label>
        </div>
      </header>

      <nav className="side-nav" aria-label="Component categories">
        <div className="nav-heading"><span>Components</span><code>gui.editor</code></div>
        {sections.map(({ id, label, icon: Icon }) => (
          <button key={id} className={activeSection === id ? "is-active" : ""} aria-current={activeSection === id ? "page" : undefined} onClick={() => navigateTo(id)}>
            <Icon size={19} weight={activeSection === id ? "fill" : "regular"} />
            <span>{label}</span>
          </button>
        ))}
        <div className="nav-footer">
          <span>Active leaf style</span>
          <code>{styleName}</code>
          <small>Same component tree · Style only</small>
        </div>
      </nav>

      <main className="content">
        <Section
          id="overview"
          eyebrow="Selected direction · Soft Workshop"
          title="A tactile workbench for serious tools"
          description="Warm machined surfaces, disciplined red accents, and instrument-like status semantics—scaled from compact desktop density to touch without changing the component tree."
          wide
        >
          <div className="overview-banner">
            <div>
              <span className="mode-chip"><Led tone="success" />Style active</span>
              <h1>{theme === "light" ? "Light" : "Dark"} · {density === "touch" ? "Touch" : "Compact"}</h1>
              <p>Four concrete leaf Styles map palette and input density onto the existing <code>Luna::GUI</code> package.</p>
            </div>
            <div className="style-matrix" aria-label="Style matrix">
              {[
                ["light.compact", "Light / Pointer"],
                ["light.touch", "Light / Touch"],
                ["dark.compact", "Dark / Pointer"],
                ["dark.touch", "Dark / Touch"],
              ].map(([name, label]) => <button key={name} className={styleName === name ? "is-active" : ""} aria-pressed={styleName === name} onClick={() => { const [nextTheme, nextDensity] = name.split("."); setTheme(nextTheme); setDensity(nextDensity === "compact" ? "pointer" : "touch"); }}><span>{label}</span><code>{name}</code></button>)}
            </div>
          </div>
          <MiniEditor
            selectedAsset={selectedAsset}
            setSelectedAsset={setSelectedAsset}
            roughness={roughness}
            setRoughness={setRoughness}
            triplanar={triplanar}
            setTriplanar={setTriplanar}
          />
        </Section>

        <Section id="primitives" eyebrow="GUITest · Primitives" title="Type, imagery, progress, and system light" description="Foundational drawing and semantic feedback, including the proposed LED primitive for tool status.">
          <div className="card-grid three-columns typography-grid">
            <Card title="Typography" note="Inter + IBM Plex Mono">
              <div className="type-stack">
                <h1>H1 · Editor title</h1><h2>H2 · Workspace title</h2><h3>H3 · Panel heading</h3>
                <h4>H4 · Property group</h4><h5>H5 · Subsection</h5><h6>H6 · Dense heading</h6>
                <p>Body · Primary application copy remains readable.</p>
                <cite>Cite · Supporting context and attribution.</cite>
                <code>Code · Float3U(1.0f, 2.0f, 3.0f)</code>
                <small>Caption · Secondary metadata · 12:45:08</small>
              </div>
            </Card>
            <Card title="Progress" note="Determinate, busy, and disabled">
              <div className="progress-stack">
                <label><span>Baking textures</span><output>68%</output><progress value="68" max="100" /></label>
                <label><span>Compiling shaders</span><output>42%</output><progress value="42" max="100" /></label>
                <label className="is-disabled"><span>Remote cache</span><output>—</output><progress value="0" max="100" /></label>
              </div>
            </Card>
            <Card title="LED language" note="Proposed design-language primitive">
              <div className="led-grid">
                <span><Led tone="success" />Online</span><span><Led tone="busy" pulse />Working</span><span><Led tone="error" />Fault</span><span><Led tone="off" />Offline</span>
                <span><Led tone="success" shape="bar" />Renderer</span><span><Led tone="busy" shape="bar" />Bake queue</span>
              </div>
            </Card>
            <Card title="Image & shape" note="Real texture assets and vector icons" className="span-two">
              <div className="image-primitives">
                <img src="/material-preview.png" alt="Material preview" />
                <div className="shape-gallery"><span><Cube size={34} weight="duotone" />Shape</span><span><ImageSquare size={34} />Image</span><span><BoundingBox size={34} />Bounds</span><span><VectorThree size={34} />Vector</span></div>
              </div>
            </Card>
            <Card title="Built-in Core icons" note="The exact 112 names embedded by Luna::GUI" className="span-three icon-catalog-card">
              <BuiltInIconCatalog />
            </Card>
          </div>
        </Section>

        <Section id="buttons" eyebrow="GUITest · Buttons" title="Actions and choices" description="Raised actions, recessed selection wells, explicit keyboard focus, and states that never depend on hover alone.">
          <div className="card-grid two-columns">
            <Card title="Button states" note="Default · hover target · active · disabled">
              <div className="button-demo-row wrap">
                <button className="button primary"><Lightning size={18} />Primary</button>
                <button className="button">Neutral</button>
                <button className="button is-pressed"><Check size={18} />Pressed</button>
                <button className="button danger"><Trash size={18} />Delete</button>
                <button className="button" disabled>Disabled</button>
                <button className="button primary" disabled>Accent disabled</button>
              </div>
              <div className="button-demo-row">
                <IconButton label="Save"><FloppyDisk size={20} /></IconButton>
                <IconButton label="Play" active><Play size={20} weight="fill" /></IconButton>
                <IconButton label="Upload"><UploadSimple size={20} /></IconButton>
                <IconButton label="Settings" disabled><GearSix size={20} /></IconButton>
              </div>
            </Card>
            <Card title="Button groups" note="Single and multiple selection">
              <Segmented items={["Build", "Run", "Profile", "Ship"]} value={groupValue} onChange={setGroupValue} />
              <Segmented items={["Build", "Run", "Profile", "Ship"]} value={multiValue} onChange={setMultiValue} multiple />
              <p className="component-caption">Selected: <code>{groupValue}</code> · Multi: <code>{multiValue.join(", ")}</code></p>
            </Card>
            <Card title="Selection controls" note="Checkbox, radio, selectable, switch">
              <div className="choice-list">
                <label><input type="checkbox" checked={checkbox} onChange={(event) => setCheckbox(event.target.checked)} />Visible in render</label>
                <label><input type="checkbox" />Receive shadows</label>
                <label className="is-disabled"><input type="checkbox" disabled />Locked by parent</label>
                {["Radio A", "Radio B"].map((item) => <label key={item}><input type="radio" name="radio-demo" checked={radio === item} onChange={() => setRadio(item)} />{item}</label>)}
                <div className="switch-row"><Toggle checked={toggle} onChange={setToggle} label="Live preview" /><span>Live preview</span><small>{toggle ? "On" : "Off"}</small></div>
              </div>
            </Card>
            <Card title="Disclosure and tree" note="Expanded, selected, leaf, arrow-only">
              <div className="disclosure-demo">
                <button className="disclosure-header" aria-expanded={disclosure} aria-controls="transform-properties" onClick={() => setDisclosure(!disclosure)}>{disclosure ? <CaretDown size={17} /> : <CaretRight size={17} />}Transform</button>
                {disclosure && <div className="disclosure-content" id="transform-properties"><p>Position <code>0.00, 1.25, 0.00</code></p><p>Rotation <code>0°, 15°, 0°</code></p></div>}
                <div className="tree-row"><CaretDown size={14} /><Folder size={16} />Scene</div>
                <div className="tree-row depth-1 is-selected"><CaretRight size={14} /><Cube size={16} />Material Ball</div>
                <div className="tree-row depth-1"><File size={16} />Camera</div>
              </div>
            </Card>
          </div>
        </Section>

        <Section id="input" eyebrow="GUITest · Input" title="Text, numeric, slider, and color editing" description="Inset fields communicate editability. Focus, read-only, disabled, and validation states remain visually distinct.">
          <div className="card-grid two-columns">
            <Card title="Text input" note="Editable · focused · read-only · disabled · error">
              <div className="field-stack">
                <label><span>Asset name</span><input defaultValue="M_Rusted_Metal" /></label>
                <label><span>Focused value</span><input className="force-focus" defaultValue="Editable text" /></label>
                <label><span>Read only</span><input readOnly value="Read-only value" /></label>
                <label><span>Disabled</span><input disabled value="Unavailable" /></label>
                <label className="has-error"><span>Source path</span><input defaultValue="/Textures/Missing.tga" aria-invalid="true" aria-describedby="source-path-error" /><small id="source-path-error">Texture could not be found.</small></label>
              </div>
            </Card>
            <Card title="Slider & drag editors" note="Scalar, integer, vector, navigation step">
              <div className="slider-stack">
                <label><span>Roughness</span><div><input type="range" aria-label="Roughness" min="0" max="100" value={sliderValue} onInput={(event) => setSliderValue(Number(event.currentTarget.value))} onChange={(event) => setSliderValue(Number(event.currentTarget.value))} /><output>{(sliderValue / 100).toFixed(2)}</output></div></label>
                <label><span>Subdivisions</span><div><input type="range" aria-label="Subdivisions" min="0" max="100" value={subdivisions} onInput={(event) => setSubdivisions(Number(event.currentTarget.value))} onChange={(event) => setSubdivisions(Number(event.currentTarget.value))} /><output>{subdivisions}</output></div></label>
                <label><span>Position</span><div className="vector-field labeled"><label>X<input defaultValue="1.000" /></label><label>Y<input defaultValue="2.000" /></label><label>Z<input defaultValue="3.000" /></label></div></label>
                <label><span>Disabled</span><div><input type="range" disabled defaultValue="30" /><output>—</output></div></label>
              </div>
            </Card>
            <Card title="Color edit" note="RGB/RGBA preview and numeric channels">
              <div className="color-editor">
                <input className="large-color" type="color" value={previewColor} onInput={(event) => setPreviewColor(event.currentTarget.value)} onChange={(event) => setPreviewColor(event.currentTarget.value)} aria-label="Color preview" />
                <div className="channel-grid"><label>R<input readOnly value={previewChannels[0]} /></label><label>G<input readOnly value={previewChannels[1]} /></label><label>B<input readOnly value={previewChannels[2]} /></label><label>A<input readOnly value="255" /></label></div>
                <code>{previewColor.toUpperCase()}FF</code>
              </div>
            </Card>
            <Card title="Combo & search" note="Popup-ready selector and filtered query">
              <div className="field-stack">
                <label><span>Shader model</span><select defaultValue="PBR Metallic Roughness"><option>PBR Metallic Roughness</option><option>Subsurface</option><option>Unlit</option></select></label>
                <label><span>Render mode</span><select defaultValue="Lit"><option>Lit</option><option>Wireframe</option><option>Normals</option></select></label>
                <div className="search-field"><MagnifyingGlass size={18} /><input placeholder="Filter components…" /><kbd>⌘ K</kbd></div>
              </div>
            </Card>
          </div>
        </Section>

        <Section id="layouts" eyebrow="GUITest · Layouts" title="Flex, grid, canvas, focus scope, and tabs" description="The visual system scales without changing layout semantics or stable element identity.">
          <div className="card-grid two-columns">
            <Card title="Flex & grid" note="Fixed, growing, and four-column cells">
              <div className="flex-demo"><button className="button">Fixed</button><button className="button grow">flex-grow: 1</button><button className="button">Fixed 140</button></div>
              <div className="grid-demo">{Array.from({ length: 8 }, (_, index) => <button key={index} className="grid-cell">Cell {index + 1}</button>)}</div>
            </Card>
            <Card title="Asset grid" note="Image, selection, metadata, and actions">
              <div className="asset-grid">
                {materialAssets.map((asset) => <button key={asset.name} className={selectedAsset === asset.name ? "is-selected" : ""} onClick={() => setSelectedAsset(asset.name)}><img src={asset.image} alt="" /><strong>{asset.name}</strong><small>{asset.meta}</small></button>)}
              </div>
            </Card>
            <Card title="Canvas anchors" note="Top-left, centered, bottom-right">
              <div className="canvas-demo"><button className="button anchor-top">Top Left</button><button className="button anchor-center">Centered</button><button className="button anchor-bottom">Bottom Right</button></div>
            </Card>
            <Card title="Tabs & focus scopes" note="Visual focus boundaries and selected-tab states">
              <div className="tab-strip">
                {["Material", "Channels", "Presets"].map((item) => <button key={item} className={activeTab === item ? "is-active" : ""} onClick={() => setActiveTab(item)}>{item}</button>)}
              </div>
              <div className="focus-scopes"><div><strong>Scope A</strong><button className="button">Item 1</button><button className="button">Item 2</button></div><div><strong>Scope B</strong><button className="button">Item 1</button><button className="button">Item 2</button></div></div>
            </Card>
          </div>
        </Section>

        <Section id="scroll" eyebrow="GUITest · Scroll Views" title="Overlay and reserved scrollbars" description="Compact mouse mode favors overlay scrollbars; touch mode keeps broad, visible affordances.">
          <div className="card-grid two-columns">
            <Card title="Dynamic overlay" note="Appears during scroll or interaction">
              <div className="scroll-demo overlay-scroll">{Array.from({ length: 16 }, (_, index) => <button key={index}>Overlay row {String(index + 1).padStart(2, "0")}</button>)}</div>
            </Card>
            <Card title="Always visible" note="Reserves viewport space">
              <div className="scroll-demo visible-scroll">{Array.from({ length: 16 }, (_, index) => <button key={index}>Persistent row {String(index + 1).padStart(2, "0")}</button>)}</div>
            </Card>
          </div>
        </Section>

        <Section id="tables" eyebrow="GUITest · Tables" title="Dense, resizable data" description="Readable rows, clear selection, subtle zebra hierarchy, and wide drag targets around thin splitters.">
          <Card title="Memory Profiler" note="Visual row states · resize-handle targets · keyboard selection">
            <div className="table-toolbar"><div className="search-field"><MagnifyingGlass size={17} /><input placeholder="Filter allocations…" /></div><button className="button"><ChartLineUp size={18} />Snapshot</button><IconButton label="Table settings"><GearSix size={18} /></IconButton></div>
            <div className="table-wrap">
              <table>
                <thead><tr><th>#<i /></th><th>Type<i /></th><th>Category<i /></th><th>Size<i /></th><th>Allocations<i /></th><th>Status</th></tr></thead>
                <tbody>{[
                  ["0001", "RHI::Texture", "Graphics", "128.0 MB", "12", "Resident", "success"],
                  ["0002", "VG::ShapeBuffer", "Vector", "48.6 MB", "64", "Resident", "success"],
                  ["0003", "Asset::Texture", "Content", "32.1 MB", "24", "Streaming", "warning"],
                  ["0004", "GUI::State", "Interface", "2.8 MB", "1,264", "Resident", "success"],
                  ["0005", "Shader Cache", "Graphics", "1.4 MB", "116", "Offline", "off"],
                ].map((row, index) => (
                  <tr
                    key={row[0]}
                    className={tableSelection === index ? "is-selected" : ""}
                    aria-selected={tableSelection === index}
                    tabIndex="0"
                    onClick={() => setTableSelection(index)}
                    onKeyDown={(event) => {
                      if (event.key === "Enter" || event.key === " ") {
                        event.preventDefault();
                        setTableSelection(index);
                      }
                    }}
                  >
                    {row.slice(0, 5).map((cell) => <td key={cell}>{cell}</td>)}
                    <td><Led tone={row[6]} />{row[5]}</td>
                  </tr>
                ))}</tbody>
              </table>
            </div>
          </Card>
        </Section>

        <Section id="overlay" eyebrow="GUITest · Overlay" title="Menus, popup, tooltip, and dialog layers" description="Transient surfaces use stronger elevation while preserving the same rounded, tactile material vocabulary.">
          <div className="card-grid two-columns">
            <Card title="Menu bar" note="Shortcut, checked item, disabled item, separator">
              <div className="menu-demo">
                <div className="menu-bar">
                  <button aria-expanded={menuOpen} aria-controls="file-menu" onClick={() => setMenuOpen(!menuOpen)}>File <CaretDown size={14} /></button><button>Edit</button><button>View</button>
                </div>
                {menuOpen && <div className="menu-popover" id="file-menu" role="menu"><button role="menuitem"><File size={17} />New <kbd>⌘N</kbd></button><button role="menuitem"><FloppyDisk size={17} />Save All <kbd>⇧⌘S</kbd></button><hr role="separator" /><button role="menuitem"><Check size={17} />Show Grid</button><button role="menuitem" disabled><LinkSimple size={17} />Unavailable</button></div>}
              </div>
            </Card>
            <Card title="Popup & tooltip" note="Independent GUICore layers in the target runtime">
              <div className="overlay-actions">
                <div className="popup-anchor"><button className="button" aria-expanded={popupOpen} aria-controls="anchored-popup" onClick={() => setPopupOpen(!popupOpen)}>Open Popup</button>{popupOpen && <div className="small-popup" id="anchored-popup" role="dialog" aria-label="Popup layer"><strong>Popup layer</strong><p>Dismiss with Done or Escape.</p><button className="button primary" onClick={() => setPopupOpen(false)}>Done</button></div>}</div>
                <span className={`tooltip-anchor ${tooltipOpen ? "is-open" : ""}`}><button className="button" aria-expanded={tooltipOpen} aria-describedby="tooltip-copy" onClick={() => setTooltipOpen(!tooltipOpen)}>Tooltip</button><span className="tooltip" id="tooltip-copy" role="tooltip">Available on hover or tap; also exposed as an accessible description.</span></span>
                <button className="button" onClick={() => setDialogOpen(true)}>Open Dialog</button>
              </div>
              {dialogOpen && <div className="dialog-backdrop"><div className="dialog-panel" role="dialog" aria-labelledby="dialog-title"><div><Warning size={23} /><h3 id="dialog-title">Remove material?</h3></div><p>This action removes the material from the current project.</p><footer><button className="button" onClick={() => setDialogOpen(false)}>Cancel</button><button className="button danger" onClick={() => setDialogOpen(false)}>Remove</button></footer></div></div>}
            </Card>
            <Card title="Combo popup" note="Alpha · Beta · Gamma · Delta">
              <select defaultValue="Alpha"><option>Alpha</option><option>Beta</option><option>Gamma</option><option>Delta</option></select>
            </Card>
            <Card title="Notifications" note="Success, warning, error, informational">
              <div className="notification-stack"><p><Led tone="success" /><strong>Build complete</strong><span>All targets are up to date.</span></p><p><Led tone="warning" /><strong>2 warnings</strong><span>Review non-manifold geometry.</span></p><p><Led tone="error" /><strong>Texture missing</strong><span>T_Decal_Warning was not found.</span></p><p><Led tone="off" /><strong>Remote sync</strong><span>Currently offline.</span></p></div>
            </Card>
          </div>
        </Section>

        <Section id="workspace" eyebrow="GUITest · Workspace" title="Docked editor composition" description="Visual target for dock tabs, splitters, floating panels, viewport tools, console, and status feedback; drag and resize behavior remains a runtime concern." wide>
          <div className="workspace-demo">
            <div className="workspace-menubar"><span>File</span><span>Edit</span><span>Create</span><span>Tools</span><span>Window</span><span>Help</span><strong>Untitled Scene.luna*</strong><StatusPill tone="success">Saved</StatusPill></div>
            <aside className="workspace-hierarchy"><div className="dock-tab"><strong>Hierarchy</strong><X size={16} /></div><div className="tree-row"><CaretDown size={14} /><Folder size={16} />Scene</div><div className="tree-row depth-1"><Camera size={16} />Camera</div><div className="tree-row depth-1 is-selected"><Cube size={16} />Material Ball</div></aside>
            <div className="workspace-viewport"><div className="dock-tabs"><button className="is-active">Viewport</button><button>Material Editor</button><button>Shader Graph</button><Plus size={16} /></div><img src="/material-preview.png" alt="Material viewport" /><div className="workspace-toolbox"><IconButton label="Select"><Selection size={18} /></IconButton><IconButton label="Translate" active><ArrowsOut size={18} /></IconButton><IconButton label="Frame"><BoundingBox size={18} /></IconButton></div></div>
            <aside className="workspace-inspector"><div className="dock-tabs"><button className="is-active">Inspector</button><button>Layers</button></div><PropertyRow label="Position"><div className="vector-field"><input aria-label="Position X" defaultValue="0.000" /><input aria-label="Position Y" defaultValue="1.250" /><input aria-label="Position Z" defaultValue="0.000" /></div></PropertyRow><PropertyRow label="Visible"><label className="check-row"><input type="checkbox" defaultChecked />Enabled</label></PropertyRow><PropertyRow label="Roughness"><div className="inline-range"><input type="range" aria-label="Workspace roughness" value={sliderValue} onInput={(event) => setSliderValue(Number(event.currentTarget.value))} onChange={(event) => setSliderValue(Number(event.currentTarget.value))} /><output>{(sliderValue / 100).toFixed(2)}</output></div></PropertyRow></aside>
            <div className="workspace-console"><div className="dock-tabs"><button className="is-active">Console</button><button>Build</button></div><p><Led tone="success" /><code>[Info]</code> Dock workspace initialized.</p><p><Led tone="warning" /><code>[Warning]</code> Texture mip chain is incomplete.</p></div>
            <div className="floating-panel"><div><strong>Inspector (Pinned)</strong><X size={16} aria-hidden="true" /></div><PropertyRow label="Scale"><div className="vector-field"><input aria-label="Scale X" defaultValue="1.000" /><input aria-label="Scale Y" defaultValue="1.000" /><input aria-label="Scale Z" defaultValue="1.000" /></div></PropertyRow><label className="check-row"><input type="checkbox" defaultChecked />Visible</label><DotsSixVertical className="resize-grip" size={18} aria-hidden="true" /></div>
          </div>
        </Section>

        <footer className="page-footer">
          <div><Cube size={22} weight="duotone" /><span><strong>Luna GUI Design Language</strong><small>Visual target prototype · no runtime code changed</small></span></div>
          <code>{styleName} · accent {accent.toUpperCase()}</code>
        </footer>
      </main>

      <aside className="token-panel">
        <div className="token-panel-title"><Palette size={20} /><div><strong>Style Inspector</strong><small>{styleName}</small></div><IconButton label="Style settings"><GearSix size={18} /></IconButton></div>
        <div className="token-section"><h3>Accent states</h3>{[
          ["Accent", "--accent", "accent"],
          ["Hover", "--accent-hover", "accent-hover"],
          ["Pressed", "--accent-active", "accent-active"],
          ["Subtle", "--accent-soft", "accent-soft"],
          ["Disabled", "--accent-disabled", "accent-disabled"],
          ["Focus", "--focus", "focus"],
        ].map(([label, token, swatch]) => <div className="token-row" key={token}><span className={`token-swatch ${swatch}`} /><span>{label}</span><code>{token}</code></div>)}</div>
        <div className="token-section"><h3>Gray hierarchy</h3>{[
          ["Surface 0", "--surface-0", "surface-0"],
          ["Surface 1", "--surface-1", "surface-1"],
          ["Surface 2", "--surface-2", "surface-2"],
          ["Surface 3", "--surface-3", "surface-3"],
          ["Surface 4", "--surface-4", "surface-4"],
          ["Surface 5", "--surface-5", "surface-5"],
          ["Text", "--text", "text"],
          ["Secondary", "--text-secondary", "text-secondary"],
          ["Muted", "--text-muted", "text-muted"],
        ].map(([label, token, swatch]) => <div className="token-row" key={token}><span className={`token-swatch ${swatch}`} /><span>{label}</span><code>{token}</code></div>)}</div>
        <div className="token-section"><h3>Density</h3><div className="metric-row"><span>Control height</span><strong>{density === "touch" ? "48" : "32"}</strong><small>lu</small></div><div className="metric-row"><span>Hit target</span><strong>{density === "touch" ? "48" : "32"}</strong><small>lu</small></div><div className="metric-row"><span>Section gap</span><strong>{density === "touch" ? "16" : "10"}</strong><small>lu</small></div></div>
        <div className="token-section"><h3>Implementation map</h3><p><span className="tag existing">Existing</span> Color, radius, font size, selected widget padding.</p><p><span className="tag proposed">Proposed</span> Full density scale, focus ring, semantic LED, custom control anatomy, shadow/elevation.</p></div>
        <div className="token-section callout"><Info size={18} /><p>Soft shadows are a visual target. Current GUICore/VG draw commands need layered primitives or a future shadow/blur decision.</p></div>
      </aside>
    </div>
  );
}
