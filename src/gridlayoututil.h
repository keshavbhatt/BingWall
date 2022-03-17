#include <QGridLayout>
#include <QWidget>

/**
 * Utility class to remove the contents of a QGridLayout row, column or
 * cell. If the deleteWidgets parameter is true, then the widgets become
 * not only removed from the layout, but also deleted. Note that we won't
 * actually remove any row or column itself from the layout, as this isn't
 * possible. So the rowCount() and columnCount() will always stay the same,
 * but the contents of the row, column or cell will be removed.
 */
class GridLayoutUtil {

public:

  // Removes the contents of the given layout row.
  static void removeRow(QGridLayout *layout, int row, bool deleteWidgets = true) {
    remove(layout, row, -1, deleteWidgets);
    layout->setRowMinimumHeight(row, 0);
    layout->setRowStretch(row, 0);
  }

  // Removes the contents of the given layout column.
  static void removeColumn(QGridLayout *layout, int column, bool deleteWidgets = true) {
    remove(layout, -1, column, deleteWidgets);
    layout->setColumnMinimumWidth(column, 0);
    layout->setColumnStretch(column, 0);
  }

  // Removes the contents of the given layout cell.
  static void removeCell(QGridLayout *layout, int row, int column, bool deleteWidgets = true) {
    remove(layout, row, column, deleteWidgets);
  }

private:

  // Removes all layout items which span the given row and column.
  static void remove(QGridLayout *layout, int row, int column, bool deleteWidgets) {
    // We avoid usage of QGridLayout::itemAtPosition() here to improve performance.
    for (int i = layout->count() - 1; i >= 0; i--) {
      int r, c, rs, cs;
      layout->getItemPosition(i, &r, &c, &rs, &cs);
      if (
          (row == -1 || (r <= row && r + rs > row)) &&
          (column == -1 || (c <= column && c + cs > column))) {
        // This layout item is subject to deletion.
        QLayoutItem *item = layout->takeAt(i);
        if (deleteWidgets) {
          deleteChildWidgets(item);
        }
        delete item;
      }
    }
  }

  // Deletes all child widgets of the given layout item.
  static void deleteChildWidgets(QLayoutItem *item) {
    QLayout *layout = item->layout();
    if (layout) {
      // Process all child items recursively.
      int itemCount = layout->count();
      for (int i = 0; i < itemCount; i++) {
        deleteChildWidgets(layout->itemAt(i));
      }
    }
    delete item->widget();
  }
};
