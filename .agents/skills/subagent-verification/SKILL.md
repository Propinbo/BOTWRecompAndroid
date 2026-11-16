---
name: subagent-verification
description: >-
  Defines when and how to dispatch independent read-only verification subagents to audit root causes, code diffs, Vulkan/memory invariants, test results, and documentation updates. Use continuously across investigations, implementation, testing, PR preparation, and doc cleanups.
---

# Independent Subagent Verification Protocol

Constantly invoke independent read-only subagents (`independent-verifier` or `research`) to avoid tunnel vision and confirm every engineering claim against primary artifacts.

## 1. Verification Checkpoints

Invoke an independent subagent at each stage:
1. **Investigation / Root Cause:** Verify static call paths, disassembly/symbol claims, and competing hypotheses before writing a fix.
2. **Pre-PR / Non-Trivial Code Diffs:** Run multi-angle adversarial reviews (e.g., 3 parallel read-only subagents for: (a) Vulkan sync/barriers/lifetimes, (b) bounds/alignment/overflow/UAF safety, (c) Mali vs. Adreno+Turnip regressions, `RT_ON_RESET`, and negative-control tests).
3. **Test & Hardware Evidence:** Verify that claimed test suites pass *after* the final edit, that mock/host tests are not conflated with physical GPU tests, and that device logs/CSV/screenshots match summary figures.
4. **Workspace & Documentation Edits:** Verify zero Cyrillic in English-only repo files, preserved hashes/SHAs, clean `git status`, and intact protected backups.

## 2. Writing Neutral, Self-Contained Verification Briefs

- Specify the exact repository/worktree path, base commit SHA, target commit/diff command, and primary log/artifact paths.
- Never bias the verifier with leading conclusions ("confirm this works"); ask it to prove or falsify the claim and report exact commands, exit codes, and discrepancies.
- For large changes, split review angles across separate subagents with disjoint checklists.

## 3. Resolving Findings

- Address every `CRITICAL` and valid `MAJOR` finding before moving on or reporting success.
- After fixing any finding, re-run affected tests and re-verify the updated diff with an independent check so no stale pass is reported.
