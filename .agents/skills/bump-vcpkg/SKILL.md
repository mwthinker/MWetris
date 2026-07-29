---
name: bump-vcpkg
description: Bump vcpkg registry hashes in App and GameServer. Use when asked to update vcpkg baselines.
---

# Bump vcpkg

From the repository root, fetch each registry's latest default-branch commit:

```bash
git ls-remote --exit-code https://github.com/microsoft/vcpkg HEAD
git ls-remote --exit-code https://github.com/mwthinker/mw-vcpkg-registry.git HEAD
```

Use each result's full 40-character hash as its `baseline` in both `App/vcpkg-configuration.json` and `GameServer/vcpkg-configuration.json`. Change nothing else.

Verify both files are valid JSON, identical, and contain the fetched hashes. Report old and new hashes. If lookup fails, do not edit either file.
