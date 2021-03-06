#ifndef TABLEBROWSER_H
#define TABLEBROWSER_H

#include "CondFormat.h"
#include "PlotDock.h"
#include "sql/Query.h"

#include <QModelIndex>
#include <QWidget>

#include <functional>
#include <map>
#include <unordered_set>
#include <vector>

class DBBrowserDB;
class ExtendedTableWidget;
class SqliteTableModel;

class QAbstractItemModel;
class QIntValidator;

namespace Ui {
class TableBrowser;
}

struct BrowseDataTableSettings
{
    using CondFormatMap = std::map<size_t, std::vector<CondFormat>>;
    std::vector<sqlb::SortedColumn> sortColumns;
    std::map<int, int> columnWidths;
    std::map<size_t, QString> filterValues;
    CondFormatMap condFormats;
    CondFormatMap rowIdFormats;
    std::map<size_t, QString> displayFormats;
    bool showRowid;
    QString encoding;
    QString plotXAxis;
    std::vector<std::map<QString, PlotDock::PlotSettings>> plotYAxes;
    QString unlockViewPk;
    std::map<int, bool> hiddenColumns;
    std::vector<QString> globalFilters;

    BrowseDataTableSettings() :
        showRowid(false),
        plotYAxes({std::map<QString, PlotDock::PlotSettings>(), std::map<QString, PlotDock::PlotSettings>()}),
        unlockViewPk("_rowid_")
    {
    }
};

class TableBrowser : public QWidget
{
    Q_OBJECT

public:
    explicit TableBrowser(QWidget* parent = nullptr);
    ~TableBrowser();
    void init(DBBrowserDB* _db);
    void reset();

    sqlb::ObjectIdentifier currentlyBrowsedTableName() const;

    std::map<sqlb::ObjectIdentifier, BrowseDataTableSettings> allSettings() const { return m_settings; }
    BrowseDataTableSettings& settings(const sqlb::ObjectIdentifier& object);
    void setSettings(const sqlb::ObjectIdentifier& table, const BrowseDataTableSettings& table_settings);

    void setStructure(QAbstractItemModel* model, const QString& old_table = QString());

    SqliteTableModel* model() { return m_model; }

    QModelIndex currentIndex() const;

    void setDefaultEncoding(const QString& encoding) { m_defaultEncoding = encoding; }
    QString defaultEncoding() const { return m_defaultEncoding; }

public slots:
    void setEnabled(bool enable);
    void updateTable();
    void clearFilters();
    void reloadSettings();
    void setCurrentTable(const sqlb::ObjectIdentifier& name);
    void updateRecordsetLabel();
    void jumpToRow(const sqlb::ObjectIdentifier& table, std::string column, const QByteArray& value);

signals:
    void projectModified();
    void selectionChanged(QModelIndex index);
    void selectionChangedByDoubleClick(QModelIndex index);
    void statusMessageRequested(QString message);
    void updatePlot(ExtendedTableWidget* tableWidget, SqliteTableModel* model, BrowseDataTableSettings* settings, bool keepOrResetSelection);
    void createView(std::string sql);
    void requestFileOpen(QString file);

private slots:
    void clear();
    void updateFilter(size_t column, const QString& value);
    void addCondFormatFromFilter(size_t column, const QString& value);
    void addCondFormat(bool isRowIdFormat, size_t column, const CondFormat& newCondFormat);
    void clearAllCondFormats(size_t column);
    void clearRowIdFormats(const QModelIndex index);
    void editCondFormats(size_t column);
    void enableEditing(bool enable_edit);
    void showRowidColumn(bool show);
    void unlockViewEditing(bool unlock, QString pk = QString());
    void hideColumns(int column = -1, bool hide = true);
    void on_actionShowAllColumns_triggered();
    void updateInsertDeleteRecordButton();
    void duplicateRecord(int currentRow);
    void headerClicked(int logicalindex);
    void updateColumnWidth(int section, int /*old_size*/, int new_size);
    void showDataColumnPopupMenu(const QPoint& pos);
    void showRecordPopupMenu(const QPoint& pos);
    void addRecord();
    void insertValues();
    void deleteRecord();
    void navigatePrevious();
    void navigateNext();
    void navigateBegin();
    void navigateEnd();
    void navigateGoto();
    void selectTableLine(int lineToSelect);
    void on_actionClearFilters_triggered();
    void on_actionClearSorting_triggered();
    void editDisplayFormat();
    void exportFilteredTable();
    void saveFilterAsView();
    void setTableEncoding(bool forAllTables = false);
    void setDefaultTableEncoding();
    void fetchedData();

private:
    enum class ReplaceMode
    {
        NoReplace,
        ReplaceNext,
        ReplaceAll,
    };
    void find(const QString& expr, bool forward, bool include_first = false, ReplaceMode replace = ReplaceMode::NoReplace);

private:
    Ui::TableBrowser* ui;
    QIntValidator* gotoValidator;
    QMenu* popupNewRecordMenu;
    QMenu* popupSaveFilterAsMenu;
    QMenu* popupHeaderMenu;

    DBBrowserDB* db;

    QAbstractItemModel* dbStructureModel;

    /// the table model used in the "Browse Data" page (re-used and
    /// re-initialized when switching to another table)
    SqliteTableModel* m_model;

    static std::map<sqlb::ObjectIdentifier, BrowseDataTableSettings> m_settings;  // This is static, so settings are shared between instances
    static QString m_defaultEncoding;

    Palette m_condFormatPalette;
    bool m_adjustRows;
    bool m_columnsResized;

    void modifySingleFormat(const bool isRowIdFormat, const QString& filter, const QModelIndex refIndex,
                            std::function<void(CondFormat&)> changeFunction);
    void modifyFormat(std::function<void(CondFormat&)> changeFunction);

    sqlb::Query buildQuery(const BrowseDataTableSettings& storedData, const sqlb::ObjectIdentifier& tablename) const;
    void applyModelSettings(const BrowseDataTableSettings& storedData, const sqlb::Query& query);
    void applyViewportSettings(const BrowseDataTableSettings& storedData, const sqlb::ObjectIdentifier& tablename);
    void generateFilters();
};

#endif
