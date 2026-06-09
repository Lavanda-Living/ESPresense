# Fork maintenance guide

This repository is a fork of [ESPresense/ESPresense](https://github.com/ESPresense/ESPresense). We maintain our own `main` branch while pulling updates from upstream, with a clear separation between upstream code and Lavanda-specific changes.

## Remotes

| Remote     | Repository                         | Purpose                          |
| ---------- | ---------------------------------- | -------------------------------- |
| `origin`   | `Lavanda-Living/ESPresense`        | Our fork — push our work here    |
| `upstream` | `ESPresense/ESPresense`            | Upstream — fetch only            |

Verify remotes:

```bash
git remote -v
```

If `upstream` is missing, add it:

```bash
git remote add upstream https://github.com/ESPresense/ESPresense.git
git fetch upstream
```

## Branch model

| Branch            | Purpose                                              | Who commits here      |
| ----------------- | ---------------------------------------------------- | --------------------- |
| `upstream/main`   | Remote-tracking ref (updated by `git fetch`)         | Upstream only         |
| `upstream-sync`   | Local mirror of upstream — always matches upstream   | Sync script / manual  |
| `main`            | Our integration branch (upstream + Lavanda changes)  | Us                    |
| `feat/*`          | Individual features or fixes                         | Us                    |

```
upstream/main
      │
      ▼  (fast-forward only)
upstream-sync
      │
      ▼  (merge)
    main  ◄── feat/my-feature
      │
      ▼
   origin/main
```

- **`upstream-sync`** is a pristine copy of upstream. Never commit Lavanda changes on it.
- **`main`** is our default branch. It contains upstream history plus our merges and feature work.
- **`feat/*`** branches are where day-to-day development happens.

## One-time setup

Create the mirror branch and prevent accidental pushes to upstream:

```bash
git fetch upstream
git branch upstream-sync upstream/main
git remote set-url --push upstream no-push
git push -u origin upstream-sync
```

## Day-to-day: Lavanda features

Always branch from `main`, not from `upstream-sync`:

```bash
git checkout main
git pull origin main
git checkout -b feat/my-feature

# ... edit, commit ...

git checkout main
git merge feat/my-feature
git push origin main
```

Keep feature branches focused. Merge them into `main` when ready; delete the branch after merge if you no longer need it.

## Syncing from upstream

Run this periodically (e.g. weekly or before starting a large feature) to pull the latest upstream `main`.

```bash
git fetch upstream

# 1. Update the mirror (must stay identical to upstream)
git checkout upstream-sync
git merge --ff-only upstream/main

# 2. Integrate into our main
git checkout main
git merge upstream-sync -m "Sync upstream/main ($(date +%Y-%m-%d))"

# 3. Resolve any merge conflicts, test, then push
git push origin main upstream-sync
```

### If fast-forward on `upstream-sync` fails

Upstream has diverged from your local mirror (rare if you never commit on `upstream-sync`). Reset the mirror to match upstream exactly:

```bash
git checkout upstream-sync
git reset --hard upstream/main
git push origin upstream-sync --force-with-lease
```

Then merge into `main` as above.

### Resolving merge conflicts

Conflicts happen when upstream and Lavanda changed the same lines. During the merge into `main`:

1. Open conflicted files and resolve markers (`<<<<<<<`, `=======`, `>>>>>>>`).
2. Prefer keeping upstream fixes unless the Lavanda change is intentional.
3. Build and test before pushing:

   ```bash
   pio-on && pio run
   cd ui && npm run build
   ```

4. Complete the merge:

   ```bash
   git add .
   git commit   # if the merge commit wasn't auto-created
   git push origin main
   ```

## Seeing what is ours vs upstream

After any sync, these commands show exactly what Lavanda added on top of upstream:

```bash
# Commits on main that are not in upstream
git log upstream-sync..main --oneline

# Full diff of our fork vs upstream
git diff upstream-sync..main

# Short stat
git diff upstream-sync..main --stat
```

Use this before and after merges to confirm only intended changes are present.

## Merge vs rebase when syncing

| Approach | Command                         | Best for                                      |
| -------- | ------------------------------- | --------------------------------------------- |
| Merge    | `git merge upstream-sync`       | Default — safe, clear sync points in history  |
| Rebase   | `git rebase upstream-sync`      | Solo use only — rewrites `main`, avoid if shared |

We use **merge** by default. Each sync produces a merge commit like `Sync upstream/main (2026-06-09)`, which makes it easy to see when upstream was integrated.

## Optional: tag sync points

For auditability, tag each successful sync:

```bash
git tag upstream-sync-$(date +%Y-%m-%d) upstream-sync
git push origin upstream-sync-$(date +%Y-%m-%d)
```

## Rules of thumb

1. **Never commit on `upstream-sync`** — only fast-forward or reset it to `upstream/main`.
2. **Never push Lavanda changes to `upstream`** unless opening a PR to contribute back.
3. **Do feature work on `feat/*`**, then merge to `main` — avoid long-lived unmerged branches.
4. **Sync before large features** — reduces painful conflict resolution later.
5. **Review the delta** — run `git diff upstream-sync..main` before pushing after a sync.

## Quick reference

```bash
# Fetch latest upstream (no local changes)
git fetch upstream

# Start a feature
git checkout main && git pull origin main
git checkout -b feat/something

# Sync upstream into our main
git fetch upstream
git checkout upstream-sync && git merge --ff-only upstream/main
git checkout main && git merge upstream-sync -m "Sync upstream/main ($(date +%Y-%m-%d))"
git push origin main upstream-sync

# What did we add?
git log upstream-sync..main --oneline
git diff upstream-sync..main --stat
```
