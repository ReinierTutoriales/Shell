# Shell — Windows 11 Integration Architecture Policy

This fork targets a fast, stable, system-adaptive Windows 11 context-menu experience while preserving Shell's scripting power.

## Non-negotiable architecture rules

1. **Windows is the default source of truth.** Theme, high contrast, transparency, DPI, menu font, system metrics and supported visual capabilities must be resolved from Windows whenever a supported API exists. NSS remains the user override layer.
2. **No cosmetic framework migration.** Do not introduce WinUI/Windows App SDK or another UI framework solely to imitate Windows 11. Extend the existing native renderer unless measurements prove a replacement is justified.
3. **No hard-coded system state in hot paths.** Windows-dependent values belong behind a system/environment resolution layer or an existing equivalent abstraction, not scattered through rendering and menu logic.
4. **Accessibility wins.** High Contrast, disabled transparency and DPI/accessibility settings override decorative effects.
5. **Transient surfaces use transient materials.** Acrylic-style treatment is appropriate for context/flyout surfaces when supported and enabled; always provide a solid fallback.
6. **System icon first.** Prefer the icon associated with the shell object/application/system resource. Custom glyphs/assets are fallback or explicit user overrides.
7. **Explorer must fail safe.** Optional styling/integration failures must degrade Shell functionality, not destabilize explorer.exe.
8. **Measure before optimizing.** Performance work must identify the affected hot path and record a before/after metric when practical: cold/warm menu latency, menu construction, NSS parsing, icon resolution, allocations, resident memory, handles/GDI objects or submenu latency.
9. **No parallel dead architecture.** A replacement is complete only when the production consumer uses it and the superseded path is removed or intentionally retained with a documented compatibility reason.
10. **Keep main releasable.** Changes on `main` must be reviewable in small coherent commits. Avoid unrelated refactors in behavior changes.

## Definition of Done

A material implementation is complete only when all applicable conditions hold:

- the production call path consumes the new implementation;
- obsolete/duplicate code introduced by the replacement is removed;
- x64 Release builds successfully (and ARM64 where the project supports it);
- relevant tests/checks pass;
- Explorer integration is smoke-tested for the changed behavior;
- light, dark, High Contrast and non-100% DPI are checked for UI changes;
- transparency-off fallback is checked for material changes;
- performance-sensitive changes include evidence that they do not regress the relevant hot path;
- the change does not silently replace user NSS overrides with forced defaults.

## Target architecture

```text
Windows
  -> System Environment / Capabilities
       -> semantic UI state (theme, colors, metrics, accessibility, icons)
            -> existing native renderer
                 <- menu model <- NSS engine
  -> Explorer integration
```

The implementation should evolve toward this structure incrementally; it must not create an unused second renderer or a disconnected design system.
