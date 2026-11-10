# Issue tracker: Local Markdown

Issues and PRDs live as Markdown files in `.scratch/`.
External PRs are not a triage surface.

## Conventions

- One feature per directory: `.scratch/<feature-slug>/`.
- PRD: `.scratch/<feature-slug>/PRD.md`.
- Implementation issues: `.scratch/<feature-slug>/issues/<NN>-<slug>.md`,
  numbered from `01`.
- Record triage state as a `Status:` line near the top.
  Use the role strings in `triage-labels.md`.
- Append comments and conversation history under `## Comments`.
- For implementation progress, keep `Status:` as the triage role and record
  `Execution: complete` or `Execution: verification-pending` separately.
  Completed code with missing acceptance evidence is verification-pending;
  do not check off criteria until the relevant evidence exists.

## Publish to the issue tracker

Create the appropriate file under `.scratch/<feature-slug>/`,
creating directories as needed.

## Fetch the relevant ticket

Read the referenced file. Resolve an issue number within its feature.
If the number matches multiple features, ask which feature is intended.

## Wayfinding operations

- Map: `.scratch/<effort>/map.md`, containing Notes,
  Decisions-so-far, and Fog.
- Child ticket: `.scratch/<effort>/issues/NN-<slug>.md`,
  numbered from `01`, with the question in the body.
- Record ticket type as `Type: research`, `prototype`, `grilling`,
  or `task`.
- For wayfinding tickets, `Status:` is the execution state:
  `open`, `claimed`, or `resolved`.
  If triage is also needed, record it separately as `Triage:`.
- Record dependencies as `Blocked by: NN, NN`.
  A ticket is unblocked when every listed ticket is `resolved`.
- Frontier: choose the lowest-numbered open, unblocked,
  unclaimed ticket.
- Claim: save `Status: claimed` before starting work.
- Resolve: append the answer under `## Answer`, save
  `Status: resolved`, then append a gist and ticket link
  to Decisions-so-far in the map.
