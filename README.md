
# Forza Horizon 2 – Drift Zones

**Add 10 custom, point-based Drift Zones to the open world of Forza Horizon 2 on PC. Inspired by the Drift Zones introduced in Forza Horizon 3.**

[![Release](https://img.shields.io/github/v/release/Philipp55112/fh2-drift-zones?include_prereleases&label=release)](https://github.com/Philipp55112/fh2-drift-zones/releases/tag/v0.9-beta)
[![Downloads](https://img.shields.io/github/downloads/Philipp55112/fh2-drift-zones/total)](https://github.com/Philipp55112/fh2-drift-zones/releases)
[![Game](https://img.shields.io/badge/game-Forza%20Horizon%202-blue)](https://github.com/Philipp55112/fh2-drift-zones)
[![Status](https://img.shields.io/badge/status-Beta-orange)](https://github.com/Philipp55112/fh2-drift-zones/releases/tag/v0.9-beta)

Drift Zones introduces a completely new type of driving challenge to Forza Horizon 2.

Instead of measuring speed, each zone evaluates your drifting performance and awards points for the Drift Skills you earn while driving through the route.

The first beta release adds **10 custom Drift Zones** across France and Italy. Every route has been driven and recorded directly in-game.

---

## Features

| Feature | Description |
|---|---|
| **10 Custom Drift Zones** | Adds 10 hand-recorded routes across France and Italy. |
| **Live Drift Scoring** | Counts Drift Skill Points while you are inside a zone. |
| **Point-Based Results** | Displays your final drift score when you reach the finish. |
| **Personal Best Tracking** | Saves an individual best score for every zone. |
| **Map Personal Best** | Displays your PB directly on the world map and results screen. |
| **Custom Map Icon** | Adds a dedicated icon for Drift Zones. |
| **Dedicated Map Filter** | Provides a separate Drift Zones category in the map filter. |
| **Available from the Start** | All 10 zones are visible on the map from the beginning. |
| **Fast Travel & Routing** | Supports Fast Travel and Route Setting just like regular Speed Zones. |
| **Start & Finish Gates** | Custom gates placed across the road, with a width of 24 metres. |
| **Localization** | Includes 14 language variants. |
| **Runtime Loading** | Loads through FH2 Extra Options without modifying original game files. |

---

## Drift Zones

The v0.9 Beta release includes the following routes:

| # | Drift Zone |
|---|---|
| 01 | Castelletto Drift Zone |
| 02 | Festival Circuit Drift Zone |
| 03 | Forest Switchbacks Drift Zone |
| 04 | Southern Coast Drift Zone |
| 05 | Forest Pass Drift Zone |
| 06 | Highland Trail Drift Zone |
| 07 | San Giovanni Loop Drift Zone |
| 08 | Montellino Switchbacks Drift Zone |
| 09 | Country Road Drift Zone |
| 10 | City Outskirts Curve Drift Zone |

All routes are distributed across France and Italy and have been recorded directly in-game.

---

## Scoring System

Drift Zones use drifting performance rather than average speed as their scoring basis.

### How it works

1. Enter a Drift Zone through its start gate.
2. Drift through the marked route.
3. Drift Skill Points are counted live while you are inside the zone.
4. Cross the finish gate to complete the run.
5. Your final result is displayed as a point total.

The results presentation follows the familiar Speed Zone concept, adapted for drift scoring.

### Personal Best System

Each Drift Zone maintains its own Personal Best.

- Individual PB tracking for every zone.
- Best score displayed directly on the world map.
- PB also displayed on the results screen.
- Automatically updated when a new personal record is achieved.

---

## World Map Integration

Drift Zones are integrated into the existing Forza Horizon 2 world map.

| Feature | Implementation |
|---|---|
| Custom Icon | Dedicated Drift Zone map icon. |
| Map Filter | Separate Drift Zones category. |
| Zone Visibility | All zones visible from the beginning. |
| Fast Travel | Supported. |
| Route Setting | Supported. |
| Personal Best | Displayed directly on the map. |

The goal is to make Drift Zones feel like a natural part of the original Forza Horizon 2 world.

---

## Localization

The mod includes 14 localized language variants:

| Language | Variant |
|---|---|
| English | United States |
| English | United Kingdom |
| German | Germany |
| French | France |
| Italian | Italy |
| Spanish | Spain |
| Spanish | Mexico |
| Portuguese | Brazil |
| Polish | Poland |
| Russian | Russia |
| Dutch | Netherlands |
| Japanese | Japan |
| Korean | South Korea |
| Chinese | Traditional |

All Drift Zone names and associated text are localized.

---

## Compatibility

| Game | Platform / Runtime | Status |
|---|---|---|
| Forza Horizon 2 | PC via Azimuth | Supported |

### Requirements

- Windows PC.
- Forza Horizon 2 (PC version).
- Compatible Azimuth installation.
- FH2 Extra Options with its runtime mod loader enabled.

This mod is designed specifically for the PC version of Forza Horizon 2 running through Azimuth.

---

## Installation

The Releases section contains the ready-to-use, prebuilt beta package.

### Installation Steps

1. Open the [Drift Zones Releases page](https://github.com/Philipp55112/fh2-drift-zones/releases).
2. Download `Driftzonen_v0.9_Beta.zip` from the Assets section.
3. Extract the ZIP archive.
4. Place the extracted mod files in the location used by your FH2 Extra Options mod-loader setup.
5. Keep the original directory structure from the archive intact.
6. Start Forza Horizon 2 through Azimuth with FH2 Extra Options and its runtime mod loader enabled.
7. Open the world map and look for the Drift Zones category.

**Important:** The exact target folder depends on your FH2 Extra Options setup. Follow the same installation convention as your other runtime mods. Do not move individual files out of the package or rename the mod components.

---

## Uninstallation

To remove Drift Zones:

1. Close Forza Horizon 2.
2. Remove the files installed from the Drift Zones package from your mod-loader location.
3. Do not delete FH2 Extra Options itself or shared files used by other mods.
4. Restart the game.

The original game files remain untouched.

---

## Technical Implementation

Drift Zones integrates into Forza Horizon 2 using custom game data and runtime components loaded by FH2 Extra Options.

### Custom Game Data

| Component | Purpose |
|---|---|
| Zone Definitions | Custom zone entries based on the game's existing Speed Zone data structure. |
| `GameObjs.xml` | Definitions for custom start and finish gate objects. |
| Map Profiles | Integration of Drift Zones into the world map. |
| Map Icons | Custom Drift Zone map icon. |
| `strings.json` | Localized names and text resources. |

### Runtime Components

| Component | Responsibility |
|---|---|
| `Driftzonen.dll` | Drift scoring, results banner and Personal Best display on the map. |
| `DriftzonenUI.dll` | Dedicated map-filter category and initial visibility of Drift Zones. |

### Runtime Loading

All components are loaded at runtime through the FH2 Extra Options mod loader.

**No original Forza Horizon 2 game files are modified.**

---

## Known Issues & Limitations

The v0.9 Beta release has a few known limitations:

| Issue | Status |
|---|---|
| Loading screen event counter may include Drift Zones in Speed Zone totals. | Under investigation |
| Some progression counters may count Drift Zones as regular Speed Zones. | Under investigation |
| Traffic and AI vehicles are not yet hidden inside Drift Zones. | Planned |
| Custom props and flags at start and finish gates are not included yet. | Planned |

The progression-counter behaviour is being investigated. The remaining items are planned improvements for future releases.

---

## Roadmap

The following improvements are planned for future versions:

| Feature | Description |
|---|---|
| AI & Traffic | Hide traffic and AI vehicles while driving inside a Drift Zone. |
| Custom Props | Add props and flags at the start and finish of each route. |
| More Drift Zones | Expand the selection of available Drift Zones. |
| Progression Fixes | Correct loading-screen and progression counters. |
| General Improvements | Refine scoring, presentation and overall behaviour. |

Further improvements will be based on feedback from beta testers and the community.

---

## Feedback & Bug Reports

This is an early beta release. Feedback from players and beta testers is highly appreciated.

When reporting an issue, please include:

- Game version and Azimuth version.
- FH2 Extra Options version.
- Name of the affected Drift Zone.
- Steps to reproduce the issue.
- Screenshots or logs, if available.

**Report bugs or submit suggestions:**

[Open an Issue](https://github.com/Philipp55112/fh2-drift-zones/issues)

**Download the latest release:**

[Drift Zones Releases](https://github.com/Philipp55112/fh2-drift-zones/releases)

---

## Credits

| Contributor | Contribution |
|---|---|
| **Philipp55112** | Project concept, implementation, integration and release. |
| **Azimuth** | PC runtime used to run Forza Horizon 2. |
| **FH2 Extra Options** | Runtime mod-loading support. |
| **Alpha Testers & Community** | Testing, feedback and suggestions. |

Special thanks to everyone who helped test the mod and provided feedback during development.

---

## Disclaimer

Drift Zones is an unofficial community-made modification.

It is not affiliated with, endorsed by or supported by Microsoft, Xbox, Playground Games or Turn 10 Studios.

Forza Horizon 2 and all related game assets, names and trademarks belong to their respective owners.

Use this modification at your own risk.

---

## Distribution & Permissions

**© 2026 Philipp55112. All rights reserved.**

No permission is granted to redistribute, modify, repackage or publish this mod or its included files without prior permission from the author.

The v0.9 Beta release is distributed as a prebuilt package. Source code is not included.

---

## Related Project

### Forza 21:9 Ultrawide Patch

A separate project by the same author, bringing true ultrawide UI support to Forza Horizon 2 and Forza Motorsport 5.

[Visit the Forza 21:9 Ultrawide Patch Repository](https://github.com/Philipp55112/fh2-ultrawide-ui)

---

**Drift Zones – Bringing a new way to drift to Forza Horizon 2.**
