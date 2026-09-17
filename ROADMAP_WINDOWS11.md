# Shell — Windows 11 Integration Roadmap

Status legend: `[ ]` pending, `[~]` in progress, `[x]` implemented and verified.

## Phase 0 — Baseline and architecture map

- [x] Establish architecture/Definition-of-Done policy.
- [~] Map the production context-menu path.
- [x] Identify the current core UI surface: `src/dll/src/ContextMenu.cpp`.
- [x] Identify current theme/DPI/system-state abstraction: `src/dll/src/Include/Theme.h`.
- [x] Identify menu model headers: `src/dll/src/Include/Menu.h`, `MenuItem.h`, `ContextMenu.h`.
- [x] Identify existing cache and hook infrastructure: `Cache.h`, `Hooker.h`.
- [ ] Map every Theme factory consumer and renderer call site.
- [x] Record a clean Release build baseline for x86/x64/ARM64 through CI.
- [ ] Record baseline menu latency/memory/handle metrics.

## Phase 1 — System Environment

- [x] Consolidate Windows theme/high-contrast/transparency state into a single `Theme::SystemState` snapshot.
- [x] Avoid repeated personalization/high-contrast registry/API reads in the paint path; capture once during menu configuration.
- [x] Preserve existing NSS overrides above system-derived defaults.
- [x] Refresh system state by capturing a new snapshot for each `ContextMenu` initialization; no persistent global visual-state cache is retained between menu instances.
- [x] Add CI architecture checks for `Theme::CaptureSystemState()` consumption and prevention of direct legacy theme-state reads from `ContextMenu`.

## Phase 2 — Semantic Windows 11 theme

- [ ] Replace duplicated Light/Dark hard-coded system defaults with semantic palette resolution.
- [ ] Keep explicit White/Black/custom themes as user choices.
- [ ] Resolve menu font from Windows; use modern Windows font only when supported and appropriate.
- [ ] Normalize Windows 11 menu radius, item radius, padding, separator and icon metrics through one token source.
- [ ] Validate 100/125/150/175/200% DPI.
- [ ] Validate High Contrast separately from light/dark.

## Phase 3 — Materials and composition

- [ ] Audit current background `effect`, tint and transparency consumers.
- [ ] Use transient/Acrylic-style material only when supported and Windows transparency is enabled.
- [ ] Provide deterministic solid fallback.
- [ ] Respect accessibility and remote/unsupported environments.
- [ ] Remove duplicate material branches after the production path is migrated.

## Phase 4 — Native icon pipeline

- [ ] Map current image/glyph/system icon resolution order.
- [ ] Prefer shell-object/application/system-provided icons.
- [ ] Cache stable icon resolution/decoded resources with bounded ownership.
- [ ] Preserve explicit NSS icon overrides.
- [ ] Normalize icon size/alignment to system/DPI metrics.
- [ ] Benchmark cold and warm icon-heavy menus.

## Phase 5 — Menu geometry and interaction

- [ ] Centralize Windows 11 menu metrics.
- [ ] Align item height, padding, icon/text gap, submenu chevrons and separators.
- [ ] Verify keyboard navigation, RTL and touch/pen-safe hit targets.
- [ ] Verify multi-monitor/per-monitor DPI transitions.
- [ ] Ensure disabled/selected/checked states remain accessible.

## Phase 6 — Explorer integration hardening

- [ ] Map hook lifecycle and Explorer entry/exit boundaries.
- [ ] Audit exception/failure containment around optional styling.
- [ ] Audit registration/unregistration and Windows 11 modern/classic menu coexistence.
- [ ] Ensure optional integration failure falls back without destabilizing Explorer.
- [ ] Stress-test repeated menu open/close and Explorer restart cycles.

## Phase 7 — Performance

- [ ] Instrument cold/warm right-click latency.
- [ ] Instrument menu construction and NSS evaluation.
- [ ] Audit repeated registry/filesystem/system API calls.
- [ ] Audit allocations in menu creation/render paths.
- [ ] Audit icon decode/cache ownership.
- [ ] Audit GDI/USER handles and resource lifetime.
- [ ] Optimize only measured bottlenecks and retain before/after evidence.

## Phase 8 — Configuration cleanup

- [ ] Separate system-derived defaults from explicit user customization.
- [ ] Keep NSS as override/configuration, not as a substitute for native system integration.
- [ ] Document the recommended modular NSS layout.
- [ ] Remove deprecated/duplicate configuration paths only after compatibility analysis.

## Phase 9 — CI and release gates

- [ ] Add reproducible Release builds for supported architectures.
- [~] Add static/build checks appropriate to the current C++ toolchain.
- [~] Add architecture regression checks where practical.
- [ ] Add smoke-test checklist for Explorer, theme, DPI and transparency.
- [ ] Add performance regression baseline.

## Current implementation order

1. Finish mapping `Theme.h` consumers and `ContextMenu.cpp` rendering/material paths.
2. Consolidate semantic Windows 11 metrics/palette.
3. Move icon resolution to system-first policy.
4. Harden Explorer boundaries.
5. Profile and optimize measured hot paths.

Do not mark an item `[x]` merely because code exists. It must satisfy `ARCHITECTURE_POLICY.md` Definition of Done.
