import {
  Archive,
  ArrowClockwise,
  ArrowCounterClockwise,
  ArrowDown,
  ArrowLeft,
  ArrowRight,
  ArrowUp,
  ArrowsClockwise,
  ArrowsCounterClockwise,
  ArrowsIn,
  ArrowsOut,
  Bell,
  BellSlash,
  Bluetooth,
  BoundingBox,
  Calendar,
  Camera,
  CaretDown,
  CaretLeft,
  CaretRight,
  CaretUp,
  Check,
  CheckCircle,
  ClipboardText,
  Clock,
  Cloud,
  Code,
  Columns,
  Command,
  Copy,
  Crop,
  Crosshair,
  Cursor,
  CursorClick,
  CursorText,
  Database,
  DotsSixVertical,
  DotsThree,
  DownloadSimple,
  Eye,
  EyeSlash,
  File,
  FileCode,
  FileImage,
  FileMinus,
  FilePlus,
  FileText,
  FilmStrip,
  FloppyDisk,
  Folder,
  FolderOpen,
  FolderPlus,
  FrameCorners,
  Funnel,
  GameController,
  GearSix,
  Globe,
  GridFour,
  HandTap,
  Heart,
  House,
  ImageSquare,
  Info,
  Keyboard,
  Lightning,
  LinkBreak,
  LinkSimple,
  ListBullets,
  Lock,
  LockOpen,
  MagnifyingGlass,
  MagnifyingGlassMinus,
  MagnifyingGlassPlus,
  MapTrifold,
  Minus,
  Moon,
  Mouse,
  MusicNote,
  Package,
  Palette,
  Pause,
  PencilSimple,
  Play,
  Plus,
  Power,
  Prohibit,
  Question,
  Record,
  Rows,
  Scissors,
  Selection,
  Sidebar,
  SlidersHorizontal,
  SpeakerHigh,
  SpeakerSlash,
  SpinnerGap,
  SquaresFour,
  Stack,
  Star,
  Stop,
  Sun,
  TerminalWindow,
  Trash,
  TreeStructure,
  UploadSimple,
  User,
  Users,
  Warning,
  WarningCircle,
  WifiHigh,
  X,
  XCircle,
} from "@phosphor-icons/react";

const iconGroups = [
  {
    name: "Navigation & layout",
    description: "Direction, disclosure, docking, view and hierarchy commands",
    icons: [
      ["arrow-up", ArrowUp], ["arrow-down", ArrowDown], ["arrow-left", ArrowLeft],
      ["arrow-right", ArrowRight], ["caret-up", CaretUp], ["caret-down", CaretDown],
      ["caret-left", CaretLeft], ["caret-right", CaretRight], ["arrow-clockwise", ArrowClockwise],
      ["arrow-counter-clockwise", ArrowCounterClockwise], ["arrows-clockwise", ArrowsClockwise],
      ["arrows-counter-clockwise", ArrowsCounterClockwise], ["arrows-in", ArrowsIn],
      ["arrows-out", ArrowsOut], ["sidebar", Sidebar], ["columns", Columns], ["rows", Rows],
      ["squares-four", SquaresFour], ["grid-four", GridFour], ["list-bullets", ListBullets],
      ["tree-structure", TreeStructure], ["stack", Stack], ["dots-three", DotsThree],
      ["dots-six-vertical", DotsSixVertical],
    ],
  },
  {
    name: "Actions & editing",
    description: "Common editing, search, visibility, persistence and link actions",
    icons: [
      ["plus", Plus], ["minus", Minus], ["x", X], ["check", Check],
      ["magnifying-glass", MagnifyingGlass], ["magnifying-glass-plus", MagnifyingGlassPlus],
      ["magnifying-glass-minus", MagnifyingGlassMinus], ["funnel", Funnel], ["gear-six", GearSix],
      ["sliders-horizontal", SlidersHorizontal], ["copy", Copy], ["scissors", Scissors],
      ["clipboard-text", ClipboardText], ["trash", Trash], ["floppy-disk", FloppyDisk],
      ["upload-simple", UploadSimple], ["download-simple", DownloadSimple], ["link-simple", LinkSimple],
      ["link-break", LinkBreak], ["pencil-simple", PencilSimple], ["eye", Eye],
      ["eye-slash", EyeSlash], ["lock", Lock], ["lock-open", LockOpen],
    ],
  },
  {
    name: "Files & content",
    description: "Files, folders, assets, code, data and command surfaces",
    icons: [
      ["file", File], ["file-text", FileText], ["file-plus", FilePlus], ["file-minus", FileMinus],
      ["file-code", FileCode], ["file-image", FileImage], ["folder", Folder],
      ["folder-open", FolderOpen], ["folder-plus", FolderPlus], ["archive", Archive],
      ["package", Package], ["image-square", ImageSquare], ["film-strip", FilmStrip],
      ["music-note", MusicNote], ["database", Database], ["cloud", Cloud], ["code", Code],
      ["terminal-window", TerminalWindow], ["command", Command], ["keyboard", Keyboard],
    ],
  },
  {
    name: "Status & media",
    description: "Semantic state, notification, playback, power and user feedback",
    icons: [
      ["info", Info], ["warning", Warning], ["question", Question], ["check-circle", CheckCircle],
      ["x-circle", XCircle], ["warning-circle", WarningCircle], ["bell", Bell],
      ["bell-slash", BellSlash], ["lightning", Lightning], ["spinner-gap", SpinnerGap],
      ["play", Play], ["pause", Pause], ["stop", Stop], ["record", Record], ["power", Power],
      ["prohibit", Prohibit], ["sun", Sun], ["moon", Moon], ["speaker-high", SpeakerHigh],
      ["speaker-slash", SpeakerSlash], ["heart", Heart], ["star", Star], ["user", User],
      ["users", Users],
    ],
  },
  {
    name: "Interaction & general",
    description: "Pointer input, selection, framing, tools, environment and platform state",
    icons: [
      ["cursor", Cursor], ["cursor-click", CursorClick], ["cursor-text", CursorText],
      ["hand-tap", HandTap], ["mouse", Mouse], ["selection", Selection],
      ["bounding-box", BoundingBox], ["frame-corners", FrameCorners], ["crop", Crop],
      ["crosshair", Crosshair], ["camera", Camera], ["palette", Palette], ["house", House],
      ["calendar", Calendar], ["clock", Clock], ["globe", Globe], ["wifi-high", WifiHigh],
      ["bluetooth", Bluetooth], ["game-controller", GameController], ["map-trifold", MapTrifold],
    ],
  },
];

const variantSamples = [
  ["regular", Folder, "regular"],
  ["bold", Check, "bold"],
  ["fill", CheckCircle, "fill"],
  ["duotone", Package, "duotone"],
];

export function BuiltInIconCatalog() {
  return (
    <div className="icon-catalog">
      <div className="icon-catalog-summary">
        <div><strong>112 Core names</strong><span>One shared VG shape buffer in Luna::GUI</span></div>
        <div className="icon-weight-samples" aria-label="Embedded icon weight examples">
          {variantSamples.map(([label, Icon, weight]) => (
            <span key={label}><Icon size={20} weight={weight} aria-hidden="true" /><small>{label}</small></span>
          ))}
        </div>
      </div>
      {iconGroups.map((group) => (
        <section className="icon-group" key={group.name} aria-label={group.name}>
          <header><h4>{group.name}</h4><p>{group.description}</p></header>
          <div className="icon-grid">
            {group.icons.map(([name, Icon]) => (
              <div className="icon-specimen" key={name} title={name}>
                <Icon size={24} weight="regular" aria-hidden="true" />
                <code>{name}</code>
              </div>
            ))}
          </div>
        </section>
      ))}
    </div>
  );
}
