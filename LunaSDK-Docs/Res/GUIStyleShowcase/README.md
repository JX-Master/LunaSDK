# Luna GUI Design Language Lab

An isolated HTML/React visual prototype for the proposed Luna GUI design language. It does not change the GUI runtime, Studio, GUITest, GUICore, or VG implementation.

## Run locally

```bash
npm install
npm run dev
```

The default local URL is printed by Vite. The validated preview in this workspace uses `http://127.0.0.1:4173/`.

## Style leaves

- `light.compact`
- `light.touch`
- `dark.compact`
- `dark.touch`

All four leaves apply to the same component tree. Theme, input density, and accent color are independent axes. The default demonstration accent is `#E34F59`.

## Coverage

The page mirrors the current GUITest showcase categories: Primitives, Buttons, Input, Layouts, Scroll Views, Tables, Overlay, and Workspace. It also includes a DCC/editor composition, semantic LED indicators, the derived accent palette, gray surface hierarchy, and responsive desktop/iPad/narrow layouts.

Soft shadows, complete density metrics, semantic LED primitives, custom control anatomy, and dock/splitter behavior are visual targets for a later runtime implementation. See `design-qa.md` for the validation record and explicit runtime deferrals.
