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

// Include
#include "constants.h"
#include "mainwindow.h"
#include "OgreMeshManager2.h"
#include "Renderwindow_dockwidget.h"
#include "paintlayer_manager.h"

//****************************************************************************/
RenderwindowDockWidget::RenderwindowDockWidget(QString title, MainWindow* parent, Qt::WindowFlags flags) : 
	QDockWidget (title, parent, flags), 
    mParent(parent),
    mButtonToggleModelAndLight(0),
    mButtonTogglePaint(0),
    mTogglePaintMode(false),
    mButtonMarker(0),
    mButtonToggleHoover(0),
    mButtonModelActive(true),
    mToggleHooverOn(false),
    mLightIcon(0),
    mPaintOnIcon(0),
    mPaintOffIcon(0),
    mModelIcon(0),
    mMarkerIcon(0),
    mHooverOnIcon(0),
    mHooverOffIcon(0)
{
    setMinimumSize(100,100);

    // Create a context menu
    installEventFilter(this);
    setContextMenuPolicy(Qt::CustomContextMenu);
    mContextMenu = new QMenu(this);
    mContextMenu->addAction(new QAction(ACTION_SET_CURRENT_MATERIAL, this));
    mMeshesSubMenu = mContextMenu->addMenu("Select mesh");
    mActionGroupMeshes = new QActionGroup(mMeshesSubMenu);
    mActionGroupMeshes->setExclusive(true);
    mContextMenu->addAction(new QAction(ACTION_TOGGLE_LIGHT_DIRECTION, this));
    mContextMenu->addAction(new QAction(ACTION_RESET_CAMERA, this));
    mContextMenu->addAction(new QAction(ACTION_TOGGLE_SUBMESH_SELECTION, this));
    mContextMenu->addAction(new QAction(ACTION_SELECT_BACKGROUND_COLOUR, this));

    // Add skyboxes to the context menu
    mSkyBoxSubMenu = mContextMenu->addMenu("Select skybox");
    mActionGroupSkyBox = new QActionGroup(mSkyBoxSubMenu);
    mActionGroupSkyBox->setExclusive(true);
    addSkyBoxNameToContextMenu (NO_SKYBOX, true);
    QSettings settings(FILE_SETTINGS, QSettings::IniFormat);
    unsigned int numberOfSkyboxes = settings.value(SETTINGS_NUMBER_OF_SKYBOXES).toInt();
    QString key;
    QString value;
    for (unsigned int i = 0; i < numberOfSkyboxes; ++i)
    {
        key = SETTINGS_PREFIX_SKYBOX + QVariant(i).toString();
        value = settings.value(key).toString();
        if (!value.isEmpty())
        {
            addSkyBoxNameToContextMenu (value, false);
        }
    }

    // Define signal and slot for the context menu
    connect(mContextMenu, SIGNAL(triggered(QAction*)), this, SLOT(contextMenuSelected(QAction*)));

    // Create the meshMap
    QFile file(QString("models.cfg"));
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream readFile(&file);
    MeshStruct meshStruct;
    QString line;
    key = "";
    QString x;
    QString y;
    QString z;
    bool first = true;
    while (!readFile.atEnd())
    {
        line = readFile.readLine();
        QStringList elements = line.split(' ', QString::SkipEmptyParts);
        if (elements.size() == 5)
        {
            // Only 5 items
            key = elements[0];
            meshStruct.meshName = elements[1];
            x = elements[2];
            y = elements[3];
            z = elements[4];
            QVector3D scale(x.toFloat(), y.toFloat(), z.toFloat());
            meshStruct.scale = scale;
            mMeshMap.insert(key, meshStruct);

            // Also add it to the context menu
            if (first)
            {
                addMeshNameToContextMenu(key, true);
                first = false;
            }
            else
                addMeshNameToContextMenu(key, false);
        }
    }

    mInnerMain = new QMainWindow();
    mInnerMain->setMinimumSize(100,100);
    setWidget(mInnerMain);

    // Perform standard functions
    createActions();
    createMenus();
    createToolBars();

    mOgreWidget = new Magus::QOgreWidget();
    mOgreWidget->setRenderwindowDockWidget(this);
    mInnerMain->setCentralWidget(mOgreWidget);
    parent->getOgreManager()->registerOgreWidget(OGRE_WIDGET_RENDERWINDOW, mOgreWidget);
    mOgreWidget->createRenderWindow(parent->getOgreManager());

    // Set the max. width of the renderwindow
    QRect rec = QApplication::desktop()->screenGeometry();
    setMaximumWidth(1.78 * rec.height()); // 1.78 = 16/9

    // Load the list with meshes
    preLoadMeshMap();

    mOgreWidget->setFocus();
}

//****************************************************************************/
RenderwindowDockWidget::~RenderwindowDockWidget(void)
{
}

//****************************************************************************/
void RenderwindowDockWidget::mousePressEventPublic( QMouseEvent* e )
{
    mousePressEvent(e);
}

//****************************************************************************/
void RenderwindowDockWidget::mousePressEvent( QMouseEvent* e )
{
    if (e->button() == Qt::RightButton)
    {
        QPoint pos;
        pos.setX(e->screenPos().x());
        pos.setY(e->screenPos().y());
        mContextMenu->popup(pos);
    }
}

//-------------------------------------------------------------------------------------
void RenderwindowDockWidget::resizeEvent(QResizeEvent *e)
{
    // The heigth of the renderwindow follows the width, because the renderwindow must alway be
    // square; this is because the render-texture for picking and highlighting is square
    // Note, that this setup makes manual resizing of the height impossible
    if(e->isAccepted())
    {
        setMinimumHeight(0.5625 * e->size().width()); // 0.5625 = 9/16
        setMaximumHeight(0.5625 * e->size().width());
    }
}

//****************************************************************************/
void RenderwindowDockWidget::addToMeshMap(const QString name,
                                          const QString meshName,
                                          QVector3D scale)
{
    MeshStruct meshStruct;
    meshStruct.meshName = meshName;
    meshStruct.scale = scale;
    mMeshMap.insert(name, meshStruct);
    QAction* mesh = new QAction(name, this);
    mMeshMenu->addAction(mesh);
    addMeshNameToContextMenu(name, true);
}

//****************************************************************************/
void RenderwindowDockWidget::preLoadMeshMap(void)
{
    Ogre::String meshName;
    QMap<QString, MeshStruct>::iterator it;
    QMap<QString, MeshStruct>::iterator itStart = mMeshMap.begin();
    QMap<QString, MeshStruct>::iterator itEnd = mMeshMap.end();
    for (it = itStart; it != itEnd; ++it)
    {
        meshName = it.value().meshName.toStdString();
        Ogre::MeshPtr v2MeshPtr = Ogre::MeshManager::getSingleton().load(
                    meshName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    }
}

//****************************************************************************/
void RenderwindowDockWidget::createActions(void)
{
    mChangeBackgroundAction = new QAction(QIcon(ICON_COLOUR), QString("Change background colour"), this);
    connect(mChangeBackgroundAction, SIGNAL(triggered()), this, SLOT(doChangeBackgroundAction()));
}

//****************************************************************************/
void RenderwindowDockWidget::createMenus(void)
{
    QAction* mesh;
    QMenuBar* menuBar = mInnerMain->menuBar();
    mMeshMenu = menuBar->addMenu(QString("Change mesh"));

    QMap<QString, MeshStruct>::iterator it;
    QMap<QString, MeshStruct>::iterator itStart = mMeshMap.begin();
    QMap<QString, MeshStruct>::iterator itEnd = mMeshMap.end();
    for (it = itStart; it != itEnd; ++it)
    {
        mesh = new QAction(it.key(), this);
        mMeshMenu->addAction(mesh);
    }
    connect(mMeshMenu, SIGNAL(triggered(QAction*)), this, SLOT(doChangeMeshAction(QAction*)));
}

//****************************************************************************/
void RenderwindowDockWidget::createToolBars(void)
{
    // Toolbar
    mHToolBar = new QToolBar();

    // Button to switch between model and light movement/rotation
    mButtonToggleModelAndLight = new QPushButton();
    mButtonTogglePaint = new QPushButton();
    mButtonMarker = new QPushButton();
    mButtonToggleHoover = new QPushButton();
    mModelIcon = new QIcon(ICON_MODEL);
    mLightIcon = new QIcon(ICON_LIGHT);
    mPaintOnIcon = new QIcon(ICON_PAINT_ON);
    mPaintOffIcon = new QIcon(ICON_PAINT_OFF);
    mMarkerIcon = new QIcon(ICON_MARKER);
    mHooverOnIcon = new QIcon(ICON_HOOVER_ON);
    mHooverOffIcon = new QIcon(ICON_HOOVER_OFF);
    mButtonToggleModelAndLight->setIcon(*mModelIcon);
    mButtonTogglePaint->setIcon(*mPaintOffIcon);
    mButtonMarker->setIcon(*mMarkerIcon);
    mButtonToggleHoover->setIcon(*mHooverOffIcon);
    connect(mButtonToggleModelAndLight, SIGNAL(clicked(bool)), this, SLOT(handleToggleModelAndLight()));
    connect(mButtonTogglePaint, SIGNAL(clicked(bool)), this, SLOT(handleTogglePaintMode()));
    connect(mButtonMarker, SIGNAL(clicked(bool)), this, SLOT(handleMarker()));
    connect(mButtonToggleHoover, SIGNAL(clicked(bool)), this, SLOT(handleToggleHoover()));

    // Transformation widget
    mTransformationWidget = new Magus::TransformationWidget(mHToolBar);
    mTransformationWidget->setMaximumWidth(344);
    mTransformationWidget->setCurrentIndex(2); // Only scale is visible
    mTransformationWidget->setListEnabled(false);
    mInnerMain->addToolBar(Qt::TopToolBarArea, mHToolBar);
    mHToolBar->setMinimumHeight(32);
    //mHToolBar->setMinimumWidth(8 * 32);
    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Add widgets
    mHToolBar->addWidget(mTransformationWidget);
    mHToolBar->addWidget(mButtonToggleModelAndLight);
    mHToolBar->addWidget(mButtonTogglePaint);
    mHToolBar->addWidget(mButtonMarker);
    mHToolBar->addWidget(mButtonToggleHoover);
    mHToolBar->addWidget(spacer);
    mHToolBar->addAction(mChangeBackgroundAction);
    connect(mTransformationWidget, SIGNAL(valueChanged()), this, SLOT(doTransformationWidgetValueChanged()));
}

//****************************************************************************/
void RenderwindowDockWidget::doChangeMeshAction(QAction* action)
{
    QMap<QString, MeshStruct>::iterator it;
    QMap<QString, MeshStruct>::iterator itStart = mMeshMap.begin();
    QMap<QString, MeshStruct>::iterator itEnd = mMeshMap.end();
    MeshStruct meshStruct;
    for (it = itStart; it != itEnd; ++it)
    {
        QString actionText = action->text();
        if (action->text() == it.key())
        {
            meshStruct = it.value();
            Ogre::Vector3 scale(meshStruct.scale.x(),
                                meshStruct.scale.y(),
                                meshStruct.scale.z());
            Ogre::String meshName = meshStruct.meshName.toStdString();
            mOgreWidget->createItem(meshName, scale);
            updateTransformationWidgetFromOgreWidget();
            setCheckedMeshNameInContextMenu(it.key()); // Point to the selected meshname in the contextmenu
        }
    }
}

//****************************************************************************/
void RenderwindowDockWidget::handleToggleModelAndLight(void)
{
    setModelAndLight(!mButtonModelActive);
}

//****************************************************************************/
void RenderwindowDockWidget::setModelAndLight(bool enabled)
{
    mButtonModelActive = enabled;
    if (mButtonModelActive)
    {
        // Disable Light axis
        mButtonToggleModelAndLight->setIcon(*mModelIcon);
        mOgreWidget->enableLightItem(false);
    }
    else
    {
        // Enable Light axis and painting is off
        mButtonToggleModelAndLight->setIcon(*mLightIcon);
        mOgreWidget->enableLightItem(true);
        setPaintMode(false);
    }
    mOgreWidget->setFocus();
}

//****************************************************************************/
void RenderwindowDockWidget::handleTogglePaintMode(void)
{
    setPaintMode(!mTogglePaintMode);
}

//****************************************************************************/
void RenderwindowDockWidget::setPaintMode(bool enabled)
{
    mTogglePaintMode = enabled;
    if (mTogglePaintMode)
    {
        // Painting is on, light axis is disabled and hoover is off
        mButtonTogglePaint->setIcon(*mPaintOnIcon);
        setModelAndLight(true);
        setHoover(false);
    }
    else
    {
        mButtonTogglePaint->setIcon(*mPaintOffIcon);
    }
    mOgreWidget->setFocus();
    mOgreWidget->setPaintMode(mTogglePaintMode);
}

//****************************************************************************/
void RenderwindowDockWidget::handleMarker(void)
{
    mOgreWidget->resetCamera();
}


//****************************************************************************/
void RenderwindowDockWidget::setHoover(bool enabled)
{
    mToggleHooverOn = enabled;
    if (mToggleHooverOn)
    {
        // If hoover is on, painting is off
        mButtonToggleHoover->setIcon(*mHooverOnIcon);
        setPaintMode(false);
    }
    else
    {
        mButtonToggleHoover->setIcon(*mHooverOffIcon);
    }
    mOgreWidget->setHoover(mToggleHooverOn);
    mOgreWidget->setFocus();
}

//****************************************************************************/
void RenderwindowDockWidget::handleToggleHoover(void)
{
    setHoover(!mToggleHooverOn);
}

//****************************************************************************/
void RenderwindowDockWidget::updateTransformationWidgetFromOgreWidget(void)
{
    if (!mOgreWidget)
        return;

    QVector3D v;

    // Scale
    v.setX(mOgreWidget->getItemScale().x);
    v.setY(mOgreWidget->getItemScale().y);
    v.setZ(mOgreWidget->getItemScale().z);
    mTransformationWidget->setScale(v);
}

//****************************************************************************/
void RenderwindowDockWidget::doTransformationWidgetValueChanged(void)
{
    if (!mOgreWidget)
        return;

    // Replace the code in this function with your own code.
    switch (mTransformationWidget->getCurrentTransformation())
    {
        case Magus::TransformationWidget::SCALE:
        {
            Ogre::Vector3 v;
            v.x = mTransformationWidget->getScale().x();
            v.y = mTransformationWidget->getScale().y();
            v.z = mTransformationWidget->getScale().z();
            mOgreWidget->setItemScale(v);
        }
        break;
    }
}

//****************************************************************************/
void RenderwindowDockWidget::doChangeBackgroundAction(void)
{
    QColorDialog dialog;
    QColor c = dialog.getColor();
    Ogre::ColourValue colour(c.red()/255.0f, c.green()/255.0f, c.blue()/255.0f, 1.0f);
    mOgreWidget->setBackgroundColour(colour);
}

//****************************************************************************/
void RenderwindowDockWidget::addMeshNameToContextMenu (const QString& meshName, bool checked)
{
    // First set all items to unchecked (only if checked is 'true')
    QAction* action;
    if (checked)
    {
        foreach (action, mActionGroupMeshes->actions())
            action->setChecked(false);
    }

    action = new QAction(meshName, this);
    action->setCheckable(true);
    action->setChecked(checked); // Set the added one to checked/unchecked
    mActionGroupMeshes->addAction(action);
    mMeshesSubMenu->addActions(mActionGroupMeshes->actions());
}

//****************************************************************************/
void RenderwindowDockWidget::setCheckedMeshNameInContextMenu (const QString& meshName)
{
    QAction* action;
    foreach (action, mActionGroupMeshes->actions())
    {
        if (action->text() == meshName)
            action->setChecked(true);
        else
            action->setChecked(false);
    }
}

//****************************************************************************/
void RenderwindowDockWidget::addSkyBoxNameToContextMenu (const QString& skyBoxName, bool checked)
{
    // First set all items to unchecked (only if checked is 'true')
    QAction* action;
    if (checked)
    {
        foreach (action, mActionGroupSkyBox->actions())
            action->setChecked(false);
    }

    action = new QAction(skyBoxName, this);
    action->setCheckable(true);
    action->setChecked(checked); // Set the added one to checked/unchecked
    mActionGroupSkyBox->addAction(action);
    mSkyBoxSubMenu->addActions(mActionGroupSkyBox->actions());
}

//****************************************************************************/
void RenderwindowDockWidget::setCheckedSkyBoxNameInContextMenu (const QString& skyBoxName)
{
    QAction* action;
    foreach (action, mActionGroupSkyBox->actions())
    {
        if (action->text() == skyBoxName)
            action->setChecked(true);
        else
            action->setChecked(false);
    }
}

//****************************************************************************/
void RenderwindowDockWidget::contextMenuSelected(QAction* action)
{
    if (action->text() == ACTION_SET_CURRENT_MATERIAL)
    {
        // TESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTEST
        PaintLayer* paintLayer;
        paintLayer = mParent->mPaintLayerManager.createPaintLayer(mOgreWidget->getCurrentDatablockName(),
                                                         Ogre::PbsTextureTypes::PBSM_DETAIL0,
                                                         "../examples/10points.png");
        paintLayer->setBrush("../common/brushes/brush_grass_01.png");
        //paintLayer->setScale(0.5);
        paintLayer->setJitterScale(0.4, 1.0);
        //paintLayer->setTranslation(-0.2, -0.2f);
        paintLayer->setJitterTranslation(-0.2, 0.2, 0.0, 0.0);
        //paintLayer->setPaintColour(Ogre::ColourValue(1.0f, 0.0f, 1.0f, 1.0f));
        //paintLayer->setJitterPaintColour(Ogre::ColourValue(0.4f, 0.1f, 0.4f, 1.0f), Ogre::ColourValue(1.0f, 0.0f, 1.0f, 1.0f));
        //paintLayer->setRotationAngle(-45);
        paintLayer->setJitterRotationAngle(-20, 20);
        paintLayer->setJitterForce(0.5, 1);
        paintLayer->setJitterMirrorHorizontal();
        paintLayer->setPaintEffect(PaintLayer::PAINT_EFFECT_TEXTURE);

        //paintLayer = mParent->mPaintLayerManager.createPaintLayer(mOgreWidget->getCurrentDatablockName(),
          //                                               Ogre::PbsTextureTypes::PBSM_DETAIL0,
            //                                             "../examples/10points.png");
        //paintLayer->setBrush("../common/brushes/brush_002.png");
        //paintLayer->setScale(0.3f);
        //paintLayer->setForce(0.2);
        //paintLayer->setRotationAngle(45);
        //paintLayer->setJitterRotationAngle(-20, 20);
        //paintLayer->setJitterTranslation(-0.1, 0.1, 0.0, 0.0);
        //paintLayer->setTranslation(0.2f, 0.0f);
        //paintLayer->setPaintEffect(PaintLayer::PAINT_EFFECT_TEXTURE);

        mOgreWidget->setPaintLayers(mParent->mPaintLayerManager.getPaintLayers());
        // TESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTEST

        mOgreWidget->assignCurrentDatablock();
        return;
    }
    else if (action->text() == ACTION_TOGGLE_LIGHT_DIRECTION)
    {
        handleToggleModelAndLight();
        return;
    }
    else if (action->text() == ACTION_RESET_CAMERA)
    {
        handleMarker();
        return;
    }
    else if (action->text() == ACTION_TOGGLE_SUBMESH_SELECTION)
    {
        handleToggleHoover();
        return;
    }
    else if (action->text() == ACTION_SELECT_BACKGROUND_COLOUR)
    {
        doChangeBackgroundAction();
        return;
    }
    else
    {
        // It could be a skybox
        if (action->text() == NO_SKYBOX)
        {
            // Make skybox node invisible
            mOgreWidget->setSkyBoxVisible(false);
            return;
        }
        else
        {
            QSettings settings(FILE_SETTINGS, QSettings::IniFormat);
            unsigned int numberOfSkyboxes = settings.value(SETTINGS_NUMBER_OF_SKYBOXES).toInt();
            QString key;
            QString value;
            for (unsigned int i = 0; i < numberOfSkyboxes; ++i)
            {
                key = SETTINGS_PREFIX_SKYBOX + QVariant(i).toString();
                value = settings.value(key).toString();
                if (value == action->text())
                {
                    // A skybox was selected; create it
                    mOgreWidget->updateSkyBoxMaterial(value.toStdString());
                    return;
                }
            }
        }
    }

    // Action is not recognized; it is probably a selected mesh
    doChangeMeshAction(action);
}
