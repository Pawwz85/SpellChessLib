# SpellChessLib

SpellChessLib is a C++17 implementation and toolkit for the Spell Chess variant on Chess.com. It implements the Spell Chess rules and provides reusable chess, engine, game-runner, and analysis tooling. The library was built primarily to support the tooling infrastructure for [Icecream engine](https://github.com/Pawwz85/icecream).

## Features

- Standard chess rules, move legality, check detection, pins, castling, en passant, and promotion.
- Spell Chess `Freeze` and `Jump` spells, including spell resources, cooldowns, frozen areas, and jump-through squares.
- Move generation and attack detection for standard and spell-aware positions.
- Standard FEN and extended Spell Chess FEN parsing and generation.
- Spell-aware move notation and UCI integration through UCILoader.
- Game arbitration, timers, players, asynchronous execution, thread pools, and concurrent game runners.
- PGN4 parsing and export, annotations, variations, game documents, and HTML rendering.
- Opening-book interfaces and Grimoire opening-book support.
- SPRT tournaments, match statistics, position suppliers, and evaluation-oriented export tooling.

The public API is under `include/SpellChess/`, with the static library target named `spellchess`.

## Building

Requirements:

- CMake 3.28 or newer.
- A C++17-compatible compiler.

Configure and build from the repository root:

```text
cmake -S . -B build
cmake --build build --config Release
```

The CMake configuration fetches the [UCILoader](https://github.com/Pawwz85/UCILoader) dependency. The unit-test configuration also fetches [GoogleTest](https://github.com/google/googletest). Network access is therefore required the first time these dependencies are configured.

Run the tests with:

```text
ctest --test-dir build --build-config Release --output-on-failure
```

For single-configuration generators, `--build-config Release` may be omitted. The project also supports the usual CMake options and generators; use a separate build directory when switching generators or configurations.

## Examples

CMake builds the following five examples:

- **TournamentRunner** (`runner`) runs engine-versus-engine SPRT tournaments and produces JSON reports.
- **Grimoire** (`grimoire`) generates Spell Chess opening books using a compatible UCI engine.
- **Pgn4Generator** (`Pgn4Generator`) generates random Spell Chess games and exports PGN4 and HTML.
- **Pgn4FenExtractor** (`Pgn4Extract`) extracts features from PGN4 files into CSV.
- **TransitionScoreExporter** (`transition_score_exporter`) searches positions with a UCI engine and exports normalized scores.

## Engine integration

[Icecream](https://github.com/Pawwz85/icecream) is a separate Spell Chess-compatible UCI engine, not part of this repository. SpellChessLib supplies the rules and infrastructure that engine supporting tools use: board state, move generation, legality checks, standard and extended FEN handling, spell-aware notation, game execution, UCI player integration, and tournament orchestration.

Ordinary coordinate moves use long-algebraic UCI-style notation, for example `e2e4` or `b7b8q` for promotion. Spell moves extend this representation with a spell action and target square; representative document notation includes `freeze@b5&c7c6`, meaning a freeze action at `b5` together with the move `c7c6`.

A standard Spell Chess starting position can be represented with extended FEN as:

```text
rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 - - F50/J20/f50/j20
```

The first six fields retain the standard FEN board, side-to-move, castling, en-passant, and move counters. The additional fields carry Spell Chess state, including spell availability and cooldown information.

## Testing and scope

The GoogleTest suite covers the core board state and hashing model, attack detection, move generation and legality, spell behavior, game execution and arbitration, timers, scheduling, thread pools, players, observers, PGN4 parsing and extraction, exporters, annotations, and Spell Chess documents. It is intended to exercise the rules engine as well as the game-runner and player infrastructure used by engine integrations.

SpellChessLib is a reusable project overview and library foundation rather than a complete API reference. The public headers and tests provide the most detailed interface and behavior examples.
