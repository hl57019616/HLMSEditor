/****************************************************************************
**
** Copyright (C) 2016
**
** This file is generated by the Magus toolkit
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
****************************************************************************/

#ifndef PAINT_LAYER_WIDGET_H
#define PAINT_LAYER_WIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QLineEdit>

QT_BEGIN_NAMESPACE

QT_END_NAMESPACE

static const int TOOL_LAYER_ICON_WIDTH = 24;
static const int TOOL_LAYER_NAME_WIDTH = 160;
static const int TOOL_LAYER_COLUMN_ICON = 0;
static const int TOOL_LAYER_COLUMN_NAME = 1;
static const int TOOL_LAYER_COLUMN_VISIBILITY = 2;
static const int TOOL_LAYER_COLUMN_FILLER = 3;
static const QString TOOL_LAYER_ACTION_CREATE_LAYER = "Create paint layer";
static const QString TOOL_LAYER_ACTION_EDIT_LAYER = "Edit paint layer";
static const QString TOOL_LAYER_ACTION_DELETE_LAYER = "Delete paint layer";
static const QString TOOL_LAYER_ACTION_RENAME_LAYER = "Rename paint layer";
static const QString TOOL_LAYER_ACTION_ALL_VISIBLE = "Make all paint layers visible";
static const QString TOOL_ICON_LAYER = QString("layer_bold.png");
static const QString TOOL_ICON_VIEW_VISIBLE = QString("view_visible_bold.png");
static const QString TOOL_ICON_VIEW_INVISIBLE = QString("view_invisible_bold.png");

/****************************************************************************
Struct for layer
***************************************************************************/
struct QtLayer
{
    int layerId;
    QString name;
    bool visible;
};

class MainWindow;
class PaintLayerDialog;
class PaintLayerDockWidget;

/****************************************************************************
Main class for paint layer widget. This widgets displays paint layers.
***************************************************************************/
class PaintLayerWidget : public QWidget
{
    Q_OBJECT

    public:
        PaintLayerWidget(const QString& iconDir, PaintLayerDockWidget* paintLayerDockWidget, QWidget* parent = 0);
        ~PaintLayerWidget(void);
        bool eventFilter(QObject* object, QEvent* event);

        // Create a new layer with default name
        void createDefaultPaintLayer(void);

        // Create a new layer
        const QtLayer* createLayer(const QString& name);

        // Create a new layer (with layerId)
        const QtLayer* createLayer(int layerId, const QString& name);

        // Add a precreated layer; the layer object is managed (and deleted) by the QtLayerWidget
        void addLayer(QtLayer* layer);

        // Edit the selected layer
        void editSelectedPaintLayer(void);

        // Delete a layer (by name)
        void deleteLayer (const QString& name);

        // Delete a layer (by pointer)
        void deleteLayer (const QtLayer* layer);

        // Delete a layer (by layerId)
        void deleteLayer (int layerId);

        // Delete all layers; it does not emit layerDeleted though
        void deleteAllLayers (void);

        // Delete the selected layer; it does not emit layerDeleted though
        void deleteSelectedPaintLayer(void);

        // Set the (name) name of the layer, identified by the (unique) layerId
        void setLayerName(int layerId, const QString& newName);

        // Get a layer (by name)
        QtLayer* getLayer(const QString& name);

        // Get a layer (by layerId)
        QtLayer* getLayer(int layerId);

        // Give list of all layers
        QVector<QtLayer*> getLayers(void);

        // Return the id of the currently selected layer
        int getCurrentLayerId(void);

        // Return the list of layers with a given name pattern
        //QVector<Layer*> findByName(const QString& namePattern);

        // Return the texture types that can be used to paint on
        QStringList getAvailableTextureTypes (void);

        // Return a vector with layer id's that are selected
        QVector<int> getSelectedLayerIds(void);

        // Set an icon in all selected layers
        void setBrushIconInCurrentLayers (const QString& brushFileName);

        // Set the visibility icon on/off
        void updateVisibilityIconForLayerId(int layerId, bool visible, bool doEmit = true);

        // Get the item based on layerId and column
        QTableWidgetItem* getItem (int layerId, int column);

    public slots:
        // Activated when a contextmenu item is selected
        void contextMenuItemSelected(QAction* action);

        // Activated when an item in the table is selected
        void tableClicked(QModelIndex index);

        // Activated when an item in the table is double clicked
        void handleTableDoubleClicked(QModelIndex index);

    signals:
        // Emitted when the visibility of a layer is enabled or disabled; layerId, name and visibility is passed
        void layerVisibiltyChanged(int layerId, const QString& name, bool visible);

        // Emitted when a layer is created or added; layerId and name of the deleted layer is passed
        void layerCreatedOrAdded(int layerId, const QString& name);

        // Emitted when a layer is deleted; layerId and name of the deleted layer is passed
        void layerDeleted(int layerId, const QString& name);

        // Emitted when a layer is selected; layerId and name of the selected layer is passed
        void layerSelected(int layerId, const QString& name);

    protected:
        void mouseClickHandler(QMouseEvent* event);
        void mouseDblClickHandler(QMouseEvent* event);
        void updateVisibilityIcon(int row, bool visible, bool doEmit = true);
        int newLayerId(void);
        int getRow(const QString& name);
        int getRow(int layerId);
        int removeFromLayerVec(int row);
        const QString& getName(int row);
        void initialisePaintLayerDialog(PaintLayerDialog* paintLayerDialog, int layerId);

    private:
        PaintLayerDockWidget* mPaintLayerDockWidget; // Reference to the dockwidget that contains this widget
        QWidget* mParent;
        QString mIconDir;
        bool mListenToDeleteEvents;
        int mListenToSceneId;
        int mLayerIdCounter;
        QVector<QtLayer*> mLayerVec; // Use this for additional data
        QTableWidget* mTable;
        QMenu* mContextMenu;
        QString mTempName;
        QVector<int> helperIntVector;
        QVector2D mHelperVector2D;
};

#endif
