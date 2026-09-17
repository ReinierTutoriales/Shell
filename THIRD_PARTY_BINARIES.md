# Third-party binary provenance

This repository vendors six static libraries used by the native Windows builds. They are active link-time dependencies and must not be treated as orphaned build artifacts.

## Source repositories

The source trees are pinned as Git submodules:

- Microsoft Detours: `src/lib/detours` at commit `4b8c659f549b0ab21cf649377c7a84eb708f5e68`
- PlutoSVG: `src/lib/plutosvg` at commit `336c02997277a1888e6ccbbbe674551a0582e5c4`
  - nested PlutoVG submodule observed at commit `91ebcfc3cca193ac9267eaf1fdff342a6583738b`

The corresponding upstream URLs are defined in `.gitmodules`.

## Vendored static libraries

The following SHA-256 values were measured in GitHub Actions on Windows Server 2022 from repository commit `08339ab04670200d7a4110a3f38f243d0ede0631`:

| Library | Size (bytes) | SHA-256 |
| --- | ---: | --- |
| `detours-arm64.lib` | 143558 | `856cf55f6aa2019e93cbb1d2bc3d3a2665e342d36d0f15e485d5d3c167690d23` |
| `detours-x64.lib` | 155108 | `e28690ba4f609c3fc9ec2ee899b25e68f42956c0751f9ab1f3ea7b4e185a8035` |
| `detours-x86.lib` | 126712 | `4b0307d209c462418e31fd399eac98a3e95b20444ba71899f8d1891f87f5079e` |
| `plutosvg-arm64.lib` | 370482 | `8085acda1cb38eb9d4b21a858f133076066778da8460f32315500368f579847a` |
| `plutosvg-x64.lib` | 417564 | `208e7ee45a990c891afe94cc1b4801d4a762db148392f0593e203fd38d03c766` |
| `plutosvg-x86.lib` | 336970 | `77d2e1703857ebb39e56e78f430c28cbcb9dd4a81144cba1bb65f0e0e232383b` |

## Current status

These libraries are linked directly by the production EXE/DLL for x86, x64, and ARM64. Their presence is intentional.

The pinned source commits and the vendored library hashes provide provenance and integrity metadata, but this does **not** yet prove bit-for-bit reproducibility. Until a deterministic rebuild procedure is established and verified, do not claim that these `.lib` files are reproducible outputs of the pinned commits.
