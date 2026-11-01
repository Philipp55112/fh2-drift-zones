Forza Horizon 2 – Drift Zones

Add 10 custom, point-based Drift Zones to the open world of Forza Horizon 2 on PC. Inspired by the Drift Zones introduced in Forza Horizon 3.

""Release" (https://img.shields.io/github/v/release/Philipp55112/fh2-drift-zones?include_prereleases&label=release)" (https://github.com/Philipp55112/fh2-drift-zones/releases/tag/v0.9-beta)
""Downloads" (https://img.shields.io/github/downloads/Philipp55112/fh2-drift-zones/total)" (https://github.com/Philipp55112/fh2-drift-zones/releases)
""Game" (https://img.shields.io/badge/game-Forza%20Horizon%202-blue)" (https://github.com/Philipp55112/fh2-drift-zones)
""Status" (https://img.shields.io/badge/status-Beta-orange)" (https://github.com/Philipp55112/fh2-drift-zones/releases/tag/v0.9-beta)

Drift Zones brings a new type of driving challenge to Forza Horizon 2. Instead of measuring speed, each zone evaluates your drifting performance and awards points for the Drift Skills you earn while driving through the route.

The first beta release adds 10 custom Drift Zones across France and Italy. Every route has been driven and recorded directly in-game.

Features

Feature| Description
10 custom Drift Zones| Adds 10 hand-recorded routes across France and Italy.
Live drift scoring| Counts Drift Skill Points while you are inside a zone.
Point-based results| Displays your final drift score when you reach the finish, using a results presentation inspired by Speed Zones.
Personal Best tracking| Saves an individual best score for every zone.
Map Personal Best| Displays your PB directly on the world map and on the results screen.
Custom map icon| Adds a dedicated icon for Drift Zones.
Dedicated map filter| Provides a separate Drift Zones category in the map filter.
Available from the start| All 10 zones are visible on the map from the beginning.
Fast Travel and routing| Supports Fast Travel and Route Setting in the same way as Speed Zones.
Start and finish gates| Uses custom gates placed across the road, with a width of 24 metres.
Localization| Includes 14 language variants.
Runtime loading| Loads through the FH2 Extra Options mod loader without modifying the original game files.

Drift Zones

The v0.9 Beta release includes the following routes:

#| Drift Zone
01| Castelletto Drift Zone
02| Festival Circuit Drift Zone
03| Forest Switchbacks Drift Zone
04| Southern Coast Drift Zone
05| Forest Pass Drift Zone
06| Highland Trail Drift Zone
07| San Giovanni Loop Drift Zone
08| Montellino Switchbacks Drift Zone
09| Country Road Drift Zone
10| City Outskirts Curve Drift Zone

Scoring and Personal Bests

Drift Zones use drift performance rather than average speed as their scoring basis.

1. Enter a Drift Zone through its start gate.
2. Drift through the marked route. Drift Skill Points are counted while you are inside the zone.
3. Cross the finish gate to complete the run.
4. Your result is displayed as a point total.

Each zone keeps its own Personal Best. The current PB is shown on the map and in the results screen, making it easy to track which routes you have improved on.

World Map Integration

Drift Zones are integrated into the existing Forza Horizon 2 world map:

- A custom Drift Zone map icon identifies each location.
- A dedicated Drift Zones map-filter category makes the new events easy to find.
- All 10 zones are available on the map from the beginning.
- Fast Travel and Route Setting work like they do for regular Speed Zones.
- The map displays the Personal Best for each completed zone.

Localization

The mod includes 14 localized language variants:

- English (United States)
- English (United Kingdom)
- German
- French
- Italian
- Spanish (Spain)
- Spanish (Mexico)
- Portuguese (Brazil)
- Polish
- Russian
- Dutch
- Japanese
- Korean
- Chinese (Traditional)

Compatibility

Game| Platform / Runtime| Status
Forza Horizon 2| PC via Azimuth with FH2 Extra Options| Supported

This mod is designed for the PC version of Forza Horizon 2 running through Azimuth. It relies on the FH2 Extra Options runtime mod loader.

Requirements

- Windows PC.
- Forza Horizon 2 (PC version) running through a compatible Azimuth installation.
- FH2 Extra Options with its runtime mod loader enabled.

Installation

The Releases section contains the ready-to-use, prebuilt beta package.

1. Open the "Drift Zones Releases page" (https://github.com/Philipp55112/fh2-drift-zones/releases).
2. Download "Driftzonen_v0.9_Beta.zip" from the Assets section of the release.
3. Extract the ZIP archive.
4. Place the extracted mod files in the location used by your FH2 Extra Options mod-loader setup. Keep the directory structure from the archive intact and follow the same installation convention as your other FH2 Extra Options mods.
5. Start Forza Horizon 2 through Azimuth with FH2 Extra Options and its runtime mod loader enabled.
6. Open the world map and look for the Drift Zones category.

Important: The exact target folder depends on your FH2 Extra Options setup. Do not move individual files out of the package or rename the mod components. If you are unsure where your loader expects mods, check the installation instructions for your FH2 Extra Options version before copying the files.

Uninstallation

To remove Drift Zones, remove the files installed from the Drift Zones package from the mod-loader location. Do not delete FH2 Extra Options itself or shared files used by other mods. Restart the game after removing the mod.

How It Works

Drift Zones is integrated into the game using custom data and runtime components loaded by FH2 Extra Options.

Custom Game Data

- Zone definitions: Custom zone entries are based on the game's existing Speed Zone data structure.
- Start and finish gates: Custom gate objects are defined through "GameObjs.xml".
- Map integration: Custom map profiles and map icons add the zones to the world map and its filters.
- Localization: Translated names and text are provided through "strings.json".

Runtime Components

Component| Responsibility
"Driftzonen.dll"| Drift scoring, results banner and Personal Best display on the map.
"DriftzonenUI.dll"| Dedicated map-filter category and visibility of the Drift Zones from the beginning.

The mod is loaded at runtime by the FH2 Extra Options mod loader. Original Forza Horizon 2 game files are not modified.

Known Issues and Limitations

- The loading-screen event counter and some progression counters may currently include Drift Zones in the Speed Zone totals.
- The traffic and AI vehicles are not yet hidden inside Drift Zones.
- Start and finish props and flags are not included in this beta release yet.

The progression-counter behaviour is being investigated. The remaining items are planned improvements rather than features of v0.9 Beta.

Roadmap

Planned improvements for future versions include:

- Hide traffic and AI vehicles while driving inside a Drift Zone.
- Add props and flags at the start and finish of each route.
- Add more Drift Zones across the map.
- Correct the loading-screen and progression counters.
- Refine scoring, presentation and overall behaviour based on beta tester feedback.

Feedback and Bug Reports

This is an early beta release. Feedback from players and beta testers is welcome.

When reporting an issue, please include:

- The game version and Azimuth version you are using.
- Your FH2 Extra Options version.
- The name of the Drift Zone where the issue occurred.
- Steps to reproduce the issue, and screenshots or logs where available.

"Open an issue on GitHub" (https://github.com/Philipp55112/fh2-drift-zones/issues) · "Download the latest release" (https://github.com/Philipp55112/fh2-drift-zones/releases)

Credits

- Philipp55112 — Project concept, implementation, integration and release.
- Azimuth — PC runtime used to run Forza Horizon 2.
- FH2 Extra Options — Runtime mod-loading support.
- Alpha testers and the community — Testing, feedback and suggestions that help improve the mod.

Disclaimer

Drift Zones is an unofficial community-made modification. It is not affiliated with, endorsed by or supported by Microsoft, Xbox, Playground Games or Turn 10 Studios.

Forza Horizon 2 and all related game assets, names and trademarks belong to their respective owners. Use this modification at your own risk.

Distribution and Permissions

© 2026 Philipp55112. All rights reserved.

No license is granted to redistribute, modify, repackage or publish this mod or its included files without prior permission from the author. The v0.9 Beta release is distributed as a prebuilt package; source code is not included.

Related Project

The "Forza 21:9 Ultrawide Patch" (https://github.com/Philipp55112/fh2-ultrawide-ui) is a separate project by the same author, bringing ultrawide UI support to Forza Horizon 2 and other Forza titles.