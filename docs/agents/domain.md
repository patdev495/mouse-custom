# Domain docs

This repository uses a single context.

## Before exploring

- Read `CONTEXT.md` at the repository root.
- Read relevant architectural decisions in `docs/adr/`.

If these files do not exist, proceed silently. Do not suggest creating
them upfront. The `domain-modeling` skill creates them lazily when
domain terms or architectural decisions are resolved.

## Layout

- `CONTEXT.md`: shared domain language and glossary.
- `docs/adr/NNNN-<decision-slug>.md`: architectural decisions.

## Use the glossary

Use the terms defined in `CONTEXT.md` in issue titles, proposals,
hypotheses, and test names. Respect explicitly avoided synonyms.
If a concept is missing, reconsider the term or note the gap for
`domain-modeling`.

## Surface ADR conflicts

Explicitly identify any existing ADR that a proposal contradicts,
and explain why the decision should be reopened.
