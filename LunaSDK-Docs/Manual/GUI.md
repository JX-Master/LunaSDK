# GUI

The `GUI` module is the low-level foundation for graphical user interfaces in LunaSDK. It provides element trees, layers, layout, input routing, reusable state and style storage, draw-command generation, and rendering. It intentionally does not provide concrete widgets.

The bundled `EditorGUI` module is the higher-level package for controls and editor-style workflows. Include `<Luna/EditorGUI/EditorGUI.hpp>` when using that package; its controls are built on the same `GUI::IContext` described by the pages below.

- [[GUI Overview]]
- [[GUI Elements and Layers]]
- [[GUI Layout]]
- [[GUI Input and Interaction]]
- [[GUI Drawing]]
- [[GUI State and Style]]
- [[GUI Performance and Inspection]]
- [[GUI World-space GUI Surfaces]]
