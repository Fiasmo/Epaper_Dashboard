# Changelog

All notable changes to **EpaperLayoutDesigner** are documented in this file.

## [0.6.3] - 2026-08-24

### Added

- Added functional layer management through the element list.
- Elements at the top of the list are displayed in front, while elements at the bottom are displayed behind other elements.
- Added **Move Forward** and **Move Backward** controls for changing the layer order of the selected element.
- Added support for saving and restoring the layer order in `.epaper` project files.
- Layer-order changes are now tracked by the Undo and Redo system.

### Improved

- Text elements now wrap automatically according to the width of their text box.
- Text that exceeds the height of its text box remains clipped.
- Automatic text wrapping is applied both to newly created text elements and to text elements loaded from existing projects.
- Element properties are preserved when changing the layer order.

