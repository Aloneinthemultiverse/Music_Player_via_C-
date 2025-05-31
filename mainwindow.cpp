#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include <QInputDialog>
#include <QRandomGenerator>
#include <QTime>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QIcon>
#include <QPixmap>
#include <QTimer>
#include <QHeaderView>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QProgressDialog>    // Added missing include
#include <QDirIterator>      // Added missing include
#include <QEventLoop>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      isPlaying(false),
      isMuted(false),
      isShuffled(false),
      repeatMode(0),
      currentIndex(-1),
      settings("MusicPlayer", "LocalMusicPlayer")
{
    // Initialize media player
    mediaPlayer = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    mediaPlayer->setAudioOutput(audioOutput);
    
    setupUi();
    setupConnections();
    setupMenus();
    loadSettings();
    
    setWindowTitle("Qt Music Player");
    resize(1000, 600);
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::setupUi()
{
    // Create central widget and main layout
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // Create tab widget
    tabWidget = new QTabWidget(this);
    
    // Now Playing Tab
    nowPlayingTab = new QWidget();
    QVBoxLayout *nowPlayingLayout = new QVBoxLayout(nowPlayingTab);
    
    // Album art and song info
    QHBoxLayout *infoLayout = new QHBoxLayout();
    
    albumArtLabel = new QLabel();
    albumArtLabel->setFixedSize(200, 200);
    albumArtLabel->setAlignment(Qt::AlignCenter);
    albumArtLabel->setStyleSheet("background-color: #f0f0f0; border: 1px solid #ddd;");
    
    QVBoxLayout *songInfoLayout = new QVBoxLayout();
    songTitleLabel = new QLabel("No song playing");
    songTitleLabel->setStyleSheet("font-size: 18pt; font-weight: bold;");
    artistLabel = new QLabel("Unknown artist");
    artistLabel->setStyleSheet("font-size: 14pt;");
    albumLabel = new QLabel("Unknown album");
    albumLabel->setStyleSheet("font-size: 12pt;");
    
    songInfoLayout->addWidget(songTitleLabel);
    songInfoLayout->addWidget(artistLabel);
    songInfoLayout->addWidget(albumLabel);
    songInfoLayout->addStretch();
    
    infoLayout->addWidget(albumArtLabel);
    infoLayout->addLayout(songInfoLayout);
    
    // Seek bar and time labels
    QHBoxLayout *seekLayout = new QHBoxLayout();
    currentTimeLabel = new QLabel("0:00");
    totalTimeLabel = new QLabel("0:00");
    seekSlider = new QSlider(Qt::Horizontal);
    
    seekLayout->addWidget(currentTimeLabel);
    seekLayout->addWidget(seekSlider);
    seekLayout->addWidget(totalTimeLabel);
    
    // Playback controls
    QHBoxLayout *controlsLayout = new QHBoxLayout();
    previousButton = new QPushButton("Previous");
    playPauseButton = new QPushButton("Play");
    stopButton = new QPushButton("Stop");
    nextButton = new QPushButton("Next");
    shuffleButton = new QPushButton("Shuffle");
    repeatButton = new QPushButton("Repeat");
    
    controlsLayout->addStretch();
    controlsLayout->addWidget(previousButton);
    controlsLayout->addWidget(playPauseButton);
    controlsLayout->addWidget(stopButton);
    controlsLayout->addWidget(nextButton);
    controlsLayout->addWidget(shuffleButton);
    controlsLayout->addWidget(repeatButton);
    controlsLayout->addStretch();
    
    // Volume controls
    QHBoxLayout *volumeLayout = new QHBoxLayout();
    muteButton = new QPushButton("Mute");
    volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(70);
    
    volumeLayout->addWidget(muteButton);
    volumeLayout->addWidget(volumeSlider);
    
    // Add all layouts to the Now Playing tab
    nowPlayingLayout->addLayout(infoLayout);
    nowPlayingLayout->addLayout(seekLayout);
    nowPlayingLayout->addLayout(controlsLayout);
    nowPlayingLayout->addLayout(volumeLayout);
    
    // Library Tab
    libraryTab = new QWidget();
    QVBoxLayout *libraryLayout = new QVBoxLayout(libraryTab);
    
    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchBox = new QLineEdit();
    searchBox->setPlaceholderText("Search library...");
    searchButton = new QPushButton("Search");
    QPushButton *scanButton = new QPushButton("Scan Library");
    
    searchLayout->addWidget(searchBox);
    searchLayout->addWidget(searchButton);
    searchLayout->addWidget(scanButton);
    
    libraryModel = new QStandardItemModel(this);
    libraryModel->setHorizontalHeaderLabels({"Title", "Artist", "Album", "Duration", "Path"});
    
    libraryTableView = new QTableView();
    libraryTableView->setModel(libraryModel);
    libraryTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    libraryTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    libraryTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    libraryTableView->verticalHeader()->setVisible(false);
    
    libraryLayout->addLayout(searchLayout);
    libraryLayout->addWidget(libraryTableView);
    
    // Playlists Tab
    playlistsTab = new QWidget();
    QVBoxLayout *playlistsLayout = new QVBoxLayout(playlistsTab);
    
    QHBoxLayout *playlistButtonsLayout = new QHBoxLayout();
    createPlaylistButton = new QPushButton("New Playlist");
    loadPlaylistButton = new QPushButton("Load Playlist");
    savePlaylistButton = new QPushButton("Save Playlist");
    
    playlistButtonsLayout->addWidget(createPlaylistButton);
    playlistButtonsLayout->addWidget(loadPlaylistButton);
    playlistButtonsLayout->addWidget(savePlaylistButton);
    
    playlistListWidget = new QListWidget();
    
    QHBoxLayout *playlistItemButtonsLayout = new QHBoxLayout();
    addToPlaylistButton = new QPushButton("Add Files");
    removeFromPlaylistButton = new QPushButton("Remove Selected");
    
    playlistItemButtonsLayout->addWidget(addToPlaylistButton);
    playlistItemButtonsLayout->addWidget(removeFromPlaylistButton);
    
    playlistsLayout->addLayout(playlistButtonsLayout);
    playlistsLayout->addWidget(playlistListWidget);
    playlistsLayout->addLayout(playlistItemButtonsLayout);
    
    // Equalizer Tab
    equalizerTab = new QWidget();
    QVBoxLayout *equalizerLayout = new QVBoxLayout(equalizerTab);
    
    QHBoxLayout *equalizerPresetsLayout = new QHBoxLayout();
    equalizerPresets = new QComboBox();
    equalizerPresets->addItems({"Flat", "Rock", "Pop", "Jazz", "Classical", "Custom"});
    saveEqualizerButton = new QPushButton("Save Preset");
    
    equalizerPresetsLayout->addWidget(new QLabel("Preset:"));
    equalizerPresetsLayout->addWidget(equalizerPresets);
    equalizerPresetsLayout->addWidget(saveEqualizerButton);
    equalizerPresetsLayout->addStretch();
    
    QHBoxLayout *slidersLayout = new QHBoxLayout();
    const QStringList bands = {"60Hz", "170Hz", "310Hz", "600Hz", "1kHz", "3kHz", "6kHz", "12kHz", "14kHz", "16kHz"};
    
    for (const QString &band : bands) {
        QVBoxLayout *bandLayout = new QVBoxLayout();
        QSlider *slider = new QSlider(Qt::Vertical);
        slider->setRange(-12, 12);
        slider->setValue(0);
        slider->setTickPosition(QSlider::TicksBothSides);
        slider->setTickInterval(3);
        equalizerSliders.append(slider);
        
        QLabel *bandLabel = new QLabel(band);
        bandLabel->setAlignment(Qt::AlignCenter);
        
        bandLayout->addWidget(slider);
        bandLayout->addWidget(bandLabel);
        slidersLayout->addLayout(bandLayout);
    }
    
    equalizerLayout->addLayout(equalizerPresetsLayout);
    equalizerLayout->addLayout(slidersLayout);
    
    // File Browser Tab
    fileBrowserTab = new QWidget();
    QVBoxLayout *fileBrowserLayout = new QVBoxLayout(fileBrowserTab);
    
    fileSystemModel = new QFileSystemModel(this);
    fileSystemModel->setRootPath(QDir::homePath());
    fileSystemModel->setFilter(QDir::AllDirs | QDir::Files);
    fileSystemModel->setNameFilters({"*.mp3", "*.wav", "*.flac", "*.ogg", "*.m4a"});
    fileSystemModel->setNameFilterDisables(false);
    
    fileSystemView = new QTreeView();
    fileSystemView->setModel(fileSystemModel);
    fileSystemView->setRootIndex(fileSystemModel->index(QDir::homePath()));
    fileSystemView->setColumnWidth(0, 250);
    fileSystemView->setAnimated(true);
    fileSystemView->setSortingEnabled(true);
    
    fileBrowserLayout->addWidget(fileSystemView);
    
    // Add all tabs to the tab widget
    tabWidget->addTab(nowPlayingTab, "Now Playing");
    tabWidget->addTab(libraryTab, "Library");
    tabWidget->addTab(playlistsTab, "Playlists");
    tabWidget->addTab(equalizerTab, "Equalizer");
    tabWidget->addTab(fileBrowserTab, "File Browser");
    
    // Add tab widget to main layout
    mainLayout->addWidget(tabWidget);
    
    // Set central widget
    setCentralWidget(centralWidget);
    
    // Set initial volume
    audioOutput->setVolume(volumeSlider->value() / 100.0);
}

void MainWindow::setupConnections()
{
    // Media player connections
    connect(mediaPlayer, &QMediaPlayer::positionChanged, this, &MainWindow::updatePosition);
    connect(mediaPlayer, &QMediaPlayer::durationChanged, this, &MainWindow::updateDuration);
    connect(mediaPlayer, &QMediaPlayer::metaDataChanged, this, &MainWindow::updateMetadata);
    
    // UI control connections
    connect(playPauseButton, &QPushButton::clicked, this, &MainWindow::playPause);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::stop);
    connect(previousButton, &QPushButton::clicked, this, &MainWindow::previous);
    connect(nextButton, &QPushButton::clicked, this, &MainWindow::next);
    connect(shuffleButton, &QPushButton::clicked, this, &MainWindow::toggleShuffle);
    connect(repeatButton, &QPushButton::clicked, this, &MainWindow::toggleRepeat);
    connect(muteButton, &QPushButton::clicked, this, &MainWindow::toggleMute);
    
    connect(seekSlider, &QSlider::sliderMoved, this, &MainWindow::seekChanged);
    connect(volumeSlider, &QSlider::valueChanged, this, &MainWindow::setVolume);
    
    // Library connections
    connect(searchButton, &QPushButton::clicked, this, &MainWindow::searchLibrary);
    
    // Playlist connections
    connect(createPlaylistButton, &QPushButton::clicked, this, &MainWindow::createPlaylist);
    connect(loadPlaylistButton, &QPushButton::clicked, this, &MainWindow::loadPlaylist);
    connect(savePlaylistButton, &QPushButton::clicked, this, &MainWindow::savePlaylist);
    connect(addToPlaylistButton, &QPushButton::clicked, this, &MainWindow::addToPlaylist);
    connect(removeFromPlaylistButton, &QPushButton::clicked, this, &MainWindow::removeFromPlaylist);
    connect(playlistListWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::playlistItemDoubleClicked);
    
    // File browser connections
    connect(fileSystemView, &QTreeView::doubleClicked, [this](const QModelIndex &index) {
        QString filePath = fileSystemModel->filePath(index);
        if (!fileSystemModel->isDir(index)) {
            loadSong(filePath);
        }
    });
}

void MainWindow::setupMenus()
{
    // File menu
    QMenu *fileMenu = menuBar()->addMenu("File");
    
    QAction *openAction = fileMenu->addAction("Open File");
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    
    QAction *scanLibraryAction = fileMenu->addAction("Scan Library");
    connect(scanLibraryAction, &QAction::triggered, this, &MainWindow::scanLibrary);
    
    fileMenu->addSeparator();
    
    QAction *exitAction = fileMenu->addAction("Exit");
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    // Playback menu
    QMenu *playbackMenu = menuBar()->addMenu("Playback");
    
    QAction *playPauseAction = playbackMenu->addAction("Play/Pause");
    connect(playPauseAction, &QAction::triggered, this, &MainWindow::playPause);
    
    QAction *stopAction = playbackMenu->addAction("Stop");
    connect(stopAction, &QAction::triggered, this, &MainWindow::stop);
    
    playbackMenu->addSeparator();
    
    QAction *sleepTimerAction = playbackMenu->addAction("Sleep Timer");
    connect(sleepTimerAction, &QAction::triggered, this, &MainWindow::setSleepTimer);
    
    // Tools menu
    QMenu *toolsMenu = menuBar()->addMenu("Tools");
    
    QAction *editMetadataAction = toolsMenu->addAction("Edit Metadata");
    connect(editMetadataAction, &QAction::triggered, this, &MainWindow::editMetadata);
    
    // Create status bar
    statusBar()->showMessage("Ready");
}

void MainWindow::loadSettings()
{
    // Load volume
    int volume = settings.value("volume", 70).toInt();
    volumeSlider->setValue(volume);
    audioOutput->setVolume(volume / 100.0);
    
    // Load last directory
    QString lastDir = settings.value("lastDirectory", QDir::homePath()).toString();
    fileSystemView->setRootIndex(fileSystemModel->index(lastDir));
    
    // Load equalizer settings
    int size = settings.beginReadArray("equalizer");
    for (int i = 0; i < size && i < equalizerSliders.size(); ++i) {
        settings.setArrayIndex(i);
        equalizerSliders[i]->setValue(settings.value("value", 0).toInt());
    }
    settings.endArray();
}

void MainWindow::saveSettings()
{
    // Save volume
    settings.setValue("volume", volumeSlider->value());
    
    // Save last directory
    QModelIndex currentIndex = fileSystemView->rootIndex();
    if (currentIndex.isValid()) {
        settings.setValue("lastDirectory", fileSystemModel->filePath(currentIndex));
    }
    
    // Save equalizer settings
    settings.beginWriteArray("equalizer");
    for (int i = 0; i < equalizerSliders.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("value", equalizerSliders[i]->value());
    }
    settings.endArray();
}

void MainWindow::openFile()
{
    QStringList filePaths = QFileDialog::getOpenFileNames(
        this,
        "Open Audio Files",
        QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first(),
        "Audio Files (*.mp3 *.wav *.flac *.ogg *.m4a);;All Files (*)"
    );
    
    if (!filePaths.isEmpty()) {
        // Add to playlist
        currentPlaylist.append(filePaths);
        updatePlaylist();
        
        // Start playing the first file if not already playing
        if (!isPlaying && currentIndex == -1) {
            currentIndex = currentPlaylist.size() - filePaths.size();
            loadSong(currentPlaylist[currentIndex]);
            playPause();
        }
    }
}

void MainWindow::playPause()
{
    if (mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        mediaPlayer->pause();
        playPauseButton->setText("Play");
        isPlaying = false;
    } else {
        if (currentIndex >= 0 && currentIndex < currentPlaylist.size()) {
            mediaPlayer->play();
            playPauseButton->setText("Pause");
            isPlaying = true;
        } else if (!currentPlaylist.isEmpty()) {
            currentIndex = 0;
            loadSong(currentPlaylist[currentIndex]);
            mediaPlayer->play();
            playPauseButton->setText("Pause");
            isPlaying = true;
        } else {
            openFile();
        }
    }
}

void MainWindow::stop()
{
    mediaPlayer->stop();
    playPauseButton->setText("Play");
    isPlaying = false;
}

void MainWindow::next()
{
    if (currentPlaylist.isEmpty()) return;
    
    if (++currentIndex >= currentPlaylist.size()) {
        if (repeatMode == 1) { // Repeat all
            currentIndex = 0;
        } else {
            currentIndex = currentPlaylist.size() - 1;
            stop();
            return;
        }
    }
    
    loadSong(currentPlaylist[currentIndex]);
    if (isPlaying) {
        mediaPlayer->play();
    }
}

void MainWindow::previous()
{
    if (currentPlaylist.isEmpty()) return;
    
    // If we're more than 3 seconds into the song, restart it
    if (mediaPlayer->position() > 3000) {
        mediaPlayer->setPosition(0);
        return;
    }
    
    if (--currentIndex < 0) {
        if (repeatMode == 1) { // Repeat all
            currentIndex = currentPlaylist.size() - 1;
        } else {
            currentIndex = 0;
            stop();
            return;
        }
    }
    
    loadSong(currentPlaylist[currentIndex]);
    if (isPlaying) {
        mediaPlayer->play();
    }
}

void MainWindow::seekChanged(int position)
{
    mediaPlayer->setPosition(position);
}

void MainWindow::updatePosition(qint64 position)
{
    seekSlider->setValue(position);
    currentTimeLabel->setText(formatTime(position));
}

void MainWindow::updateDuration(qint64 duration)
{
    seekSlider->setRange(0, duration);
    totalTimeLabel->setText(formatTime(duration));
}

void MainWindow::updateMetadata()
{
    // Get metadata from the media player
    QMediaMetaData metaData = mediaPlayer->metaData();
    
    // Update song info
    QString title = metaData.value(QMediaMetaData::Title).toString();
    if (title.isEmpty() && currentIndex >= 0 && currentIndex < currentPlaylist.size()) {
        QFileInfo fileInfo(currentPlaylist[currentIndex]);
        title = fileInfo.baseName();
    }
    songTitleLabel->setText(title.isEmpty() ? "Unknown Title" : title);
    
    QString artist = metaData.value(QMediaMetaData::AlbumArtist).toString();
    if (artist.isEmpty()) {
        artist = metaData.value(QMediaMetaData::Author).toString();
    }
    artistLabel->setText(artist.isEmpty() ? "Unknown Artist" : artist);
    
    QString album = metaData.value(QMediaMetaData::AlbumTitle).toString();
    albumLabel->setText(album.isEmpty() ? "Unknown Album" : album);
    
    // Update album art
    QVariant coverArtVariant = metaData.value(QMediaMetaData::CoverArtImage);
    if (coverArtVariant.isValid()) {
        QImage coverArt = coverArtVariant.value<QImage>();
        if (!coverArt.isNull()) {
            QPixmap pixmap = QPixmap::fromImage(coverArt);
            albumArtLabel->setPixmap(pixmap.scaled(albumArtLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            albumArtLabel->setText("No Cover");
        }
    } else {
        albumArtLabel->setText("No Cover");
    }
    
    // Store metadata for later use
    currentMetadata.clear();
    currentMetadata["title"] = title;
    currentMetadata["artist"] = artist;
    currentMetadata["album"] = album;
}

void MainWindow::setVolume(int volume)
{
    audioOutput->setVolume(volume / 100.0);
}

void MainWindow::toggleMute()
{
    isMuted = !isMuted;
    audioOutput->setMuted(isMuted);
    muteButton->setText(isMuted ? "Unmute" : "Mute");
}

void MainWindow::toggleRepeat()
{
    repeatMode = (repeatMode + 1) % 3;
    
    switch (repeatMode) {
    case 0:
        repeatButton->setText("Repeat");
        break;
    case 1:
        repeatButton->setText("Repeat All");
        break;
    case 2:
        repeatButton->setText("Repeat One");
        break;
    }
}

void MainWindow::toggleShuffle()
{
    isShuffled = !isShuffled;
    shuffleButton->setText(isShuffled ? "Unshuffle" : "Shuffle");
    
    if (isShuffled) {
        shufflePlaylist();
    } else {
        // TODO: Restore original order
    }
}

void MainWindow::createPlaylist()
{
    bool ok;
    QString name = QInputDialog::getText(this, "New Playlist", "Playlist name:", QLineEdit::Normal, "", &ok);
    
    if (ok && !name.isEmpty()) {
        // Clear current playlist
        currentPlaylist.clear();
        updatePlaylist();
        
        // Set window title to include playlist name
        setWindowTitle("Qt Music Player - " + name);
    }
}

void MainWindow::loadPlaylist()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Open Playlist",
        QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first(),
        "Playlist Files (*.m3u *.pls);;All Files (*)"
    );
    
    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            currentPlaylist.clear();
            
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (!line.isEmpty() && !line.startsWith('#')) {
                    currentPlaylist.append(line);
                }
            }
            
            file.close();
            updatePlaylist();
            
            // Set window title to include playlist name
            QFileInfo fileInfo(filePath);
            setWindowTitle("Qt Music Player - " + fileInfo.baseName());
        }
    }
}

void MainWindow::savePlaylist()
{
    if (currentPlaylist.isEmpty()) {
        QMessageBox::information(this, "Save Playlist", "The playlist is empty.");
        return;
    }
    
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save Playlist",
        QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first(),
        "M3U Playlist (*.m3u);;PLS Playlist (*.pls)"
    );
    
    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            
            if (filePath.endsWith(".m3u", Qt::CaseInsensitive)) {
                out << "#EXTM3U\n";
                for (const QString &path : currentPlaylist) {
                    out << path << "\n";
                }
            } else if (filePath.endsWith(".pls", Qt::CaseInsensitive)) {
                out << "[playlist]\n";
                out << "NumberOfEntries=" << currentPlaylist.size() << "\n";
                
                for (int i = 0; i < currentPlaylist.size(); ++i) {
                    out << "File" << (i+1) << "=" << currentPlaylist[i] << "\n";
                    out << "Title" << (i+1) << "=" << QFileInfo(currentPlaylist[i]).baseName() << "\n";
                    out << "Length" << (i+1) << "=-1\n";
                }
                
                out << "Version=2\n";
            }
            
            file.close();
            QMessageBox::information(this, "Save Playlist", "Playlist saved successfully.");
        }
    }
}

void MainWindow::addToPlaylist()
{
    QStringList filePaths = QFileDialog::getOpenFileNames(
        this,
        "Add Files to Playlist",
        QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first(),
        "Audio Files (*.mp3 *.wav *.flac *.ogg *.m4a);;All Files (*)"
    );
    
    if (!filePaths.isEmpty()) {
        currentPlaylist.append(filePaths);
        updatePlaylist();
    }
}

void MainWindow::removeFromPlaylist()
{
    QList<QListWidgetItem*> selectedItems = playlistListWidget->selectedItems();
    if (selectedItems.isEmpty()) {
        return;
    }
    
    for (QListWidgetItem *item : selectedItems) {
        int row = playlistListWidget->row(item);
        if (row >= 0 && row < currentPlaylist.size()) {
            currentPlaylist.removeAt(row);
            delete playlistListWidget->takeItem(row);
        }
    }
    
    // Update current index if necessary
    if (currentIndex >= currentPlaylist.size()) {
        currentIndex = currentPlaylist.size() - 1;
    }
}

void MainWindow::playlistItemDoubleClicked(QListWidgetItem *item)
{
    int row = playlistListWidget->row(item);
    if (row >= 0 && row < currentPlaylist.size()) {
        currentIndex = row;
        loadSong(currentPlaylist[currentIndex]);
        mediaPlayer->play();
        playPauseButton->setText("Pause");
        isPlaying = true;
    }
}

void MainWindow::searchLibrary()
{
    QString searchText = searchBox->text().toLower();
    if (searchText.isEmpty()) {
        return;
    }
    
    for (int row = 0; row < libraryModel->rowCount(); ++row) {
        bool match = false;
        
        for (int col = 0; col < 3; ++col) { // Check title, artist, album
            QStandardItem *item = libraryModel->item(row, col);
            if (item && item->text().toLower().contains(searchText)) {
                match = true;
                break;
            }
        }
        
        libraryTableView->setRowHidden(row, !match);
    }
}

void MainWindow::scanLibrary()
{
    QString musicDir = QFileDialog::getExistingDirectory(
        this,
        "Select Music Directory",
        QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first()
        );

    if (musicDir.isEmpty()) return;

    // Clear existing library
    libraryModel->removeRows(0, libraryModel->rowCount());

    // Show progress dialog (fixed declaration)
    QProgressDialog progress("Scanning music library...", "Cancel", 0, 100, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();

    // Fixed QDirIterator usage
    QDirIterator it(musicDir,
                    QStringList() << "*.mp3" << "*.wav" << "*.flac" << "*.ogg" << "*.m4a",
                    QDir::Files,
                    QDirIterator::Subdirectories);  // Fixed enum

    QStringList files;
    while (it.hasNext()) {
        files.append(it.next());
    }

    int total = files.size();
    int current = 0;

    for (const QString &filePath : files) {
        // Update progress
        progress.setValue(current * 100 / total);
        if (progress.wasCanceled()) break;

        // Fixed event loop implementation
        QMediaPlayer tempPlayer;
        tempPlayer.setSource(QUrl::fromLocalFile(filePath));

        QEventLoop loop;
        QTimer::singleShot(100, &loop, &QEventLoop::quit);
        loop.exec();




        // Get metadata
        QMediaMetaData metaData = tempPlayer.metaData();
        
        QString title = metaData.value(QMediaMetaData::Title).toString();
        if (title.isEmpty()) {
            QFileInfo fileInfo(filePath);
            title = fileInfo.baseName();
        }
        
        QString artist = metaData.value(QMediaMetaData::AlbumArtist).toString();
        if (artist.isEmpty()) {
            artist = metaData.value(QMediaMetaData::Author).toString();
        }
        if (artist.isEmpty()) {
            artist = "Unknown Artist";
        }
        
        QString album = metaData.value(QMediaMetaData::AlbumTitle).toString();
        if (album.isEmpty()) {
            album = "Unknown Album";
        }
        
        qint64 duration = tempPlayer.duration();
        QString durationStr = formatTime(duration);
        
        // Add to library model
        QList<QStandardItem*> row;
        row.append(new QStandardItem(title));
        row.append(new QStandardItem(artist));
        row.append(new QStandardItem(album));
        row.append(new QStandardItem(durationStr));
        row.append(new QStandardItem(filePath));
        
        libraryModel->appendRow(row);
        
        current++;
    }
    
    progress.setValue(100);
    
    // Connect double-click on library item to play
    connect(libraryTableView, &QTableView::doubleClicked, [this](const QModelIndex &index) {
        int row = index.row();
        QString filePath = libraryModel->item(row, 4)->text(); // Path is in column 4
        
        // Add to playlist if not already there
        if (!currentPlaylist.contains(filePath)) {
            currentPlaylist.append(filePath);
            updatePlaylist();
        }
        
        // Set current index and play
        currentIndex = currentPlaylist.indexOf(filePath);
        loadSong(filePath);
        mediaPlayer->play();
        playPauseButton->setText("Pause");
        isPlaying = true;
    });
    
    statusBar()->showMessage(QString("Library scan complete: %1 files found").arg(libraryModel->rowCount()));
}

void MainWindow::editMetadata()
{
    if (currentIndex < 0 || currentIndex >= currentPlaylist.size()) {
        QMessageBox::information(this, "Edit Metadata", "No song is currently selected.");
        return;
    }
    
    QString filePath = currentPlaylist[currentIndex];
    
    // Create dialog
    QDialog dialog(this);
    dialog.setWindowTitle("Edit Metadata");
    
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    
    QFormLayout *formLayout = new QFormLayout();
    
    QLineEdit *titleEdit = new QLineEdit(currentMetadata.value("title").toString());
    QLineEdit *artistEdit = new QLineEdit(currentMetadata.value("artist").toString());
    QLineEdit *albumEdit = new QLineEdit(currentMetadata.value("album").toString());
    
    formLayout->addRow("Title:", titleEdit);
    formLayout->addRow("Artist:", artistEdit);
    formLayout->addRow("Album:", albumEdit);
    
    layout->addLayout(formLayout);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    layout->addWidget(buttonBox);
    
    if (dialog.exec() == QDialog::Accepted) {
        // TODO: Save metadata to file
        // This requires a tag library like TagLib which is not part of Qt
        // For now, just update the current metadata in memory
        currentMetadata["title"] = titleEdit->text();
        currentMetadata["artist"] = artistEdit->text();
        currentMetadata["album"] = albumEdit->text();
        
        // Update UI
        songTitleLabel->setText(titleEdit->text());
        artistLabel->setText(artistEdit->text());
        albumLabel->setText(albumEdit->text());
        
        QMessageBox::information(this, "Edit Metadata", 
            "Metadata editing is not fully implemented in this example.\n"
            "To implement this feature completely, you would need to use a tag library like TagLib.");
    }
}

void MainWindow::applyEqualizer(int band, int value)
{
    // TODO: Implement equalizer functionality
    // This would require audio processing which is not directly supported in Qt
    // You would need to use a library like FFmpeg or implement DSP algorithms
}

void MainWindow::saveEqualizerPreset()
{
    bool ok;
    QString name = QInputDialog::getText(this, "Save Equalizer Preset", 
                                         "Preset name:", QLineEdit::Normal, "Custom", &ok);
    
    if (ok && !name.isEmpty()) {
        // Save equalizer settings
        QList<int> values;
        for (QSlider *slider : equalizerSliders) {
            values.append(slider->value());
        }
        
        settings.beginWriteArray("equalizerPreset_" + name);
        for (int i = 0; i < values.size(); ++i) {
            settings.setArrayIndex(i);
            settings.setValue("value", values[i]);
        }
        settings.endArray();
        
        // Add to presets combobox if not already there
        if (equalizerPresets->findText(name) == -1) {
            equalizerPresets->addItem(name);
        }
        
        equalizerPresets->setCurrentText(name);
    }
}

void MainWindow::loadEqualizerPreset()
{
    QString name = equalizerPresets->currentText();
    
    if (name == "Flat") {
        for (QSlider *slider : equalizerSliders) {
            slider->setValue(0);
        }
    } else if (name == "Rock") {
        int values[] = {4, 3, 2, 0, -1, -1, 2, 3, 4, 4};
        for (int i = 0; i < 10 && i < equalizerSliders.size(); ++i) {
            equalizerSliders[i]->setValue(values[i]);
        }
    } else if (name == "Pop") {
        int values[] = {-2, -1, 0, 2, 4, 4, 2, 0, -1, -2};
        for (int i = 0; i < 10 && i < equalizerSliders.size(); ++i) {
            equalizerSliders[i]->setValue(values[i]);
        }
    } else if (name == "Jazz") {
        int values[] = {0, 0, 1, 3, 3, 3, 3, 1, 1, 1};
        for (int i = 0; i < 10 && i < equalizerSliders.size(); ++i) {
            equalizerSliders[i]->setValue(values[i]);
        }
    } else if (name == "Classical") {
        int values[] = {5, 4, 3, 3, 0, 0, 0, 3, 4, 5};
        for (int i = 0; i < 10 && i < equalizerSliders.size(); ++i) {
            equalizerSliders[i]->setValue(values[i]);
        }
    } else {
        // Load custom preset
        int size = settings.beginReadArray("equalizerPreset_" + name);
        for (int i = 0; i < size && i < equalizerSliders.size(); ++i) {
            settings.setArrayIndex(i);
            equalizerSliders[i]->setValue(settings.value("value", 0).toInt());
        }
        settings.endArray();
    }
    
    // Apply equalizer settings
    for (int i = 0; i < equalizerSliders.size(); ++i) {
        applyEqualizer(i, equalizerSliders[i]->value());
    }
}

void MainWindow::setSleepTimer()
{
    QStringList options = {"Off", "15 minutes", "30 minutes", "45 minutes", "60 minutes", "90 minutes"};
    
    bool ok;
    QString selected = QInputDialog::getItem(this, "Sleep Timer", 
                                            "Stop playback after:", options, 0, false, &ok);
    
    if (ok && !selected.isEmpty()) {
        if (selected == "Off") {
            // Cancel any existing timer
            QTimer::singleShot(0, [](){});
            statusBar()->showMessage("Sleep timer disabled");
        } else {
            // Extract minutes
            int minutes = selected.split(" ")[0].toInt();
            int ms = minutes * 60 * 1000;
            
            // Set timer
            QTimer::singleShot(ms, [this](){
                stop();
                QMessageBox::information(this, "Sleep Timer", "Sleep timer activated. Playback stopped.");
            });
            
            statusBar()->showMessage(QString("Sleep timer set for %1").arg(selected));
        }
    }
}

QString MainWindow::formatTime(qint64 ms)
{
    int seconds = ms / 1000;
    int minutes = seconds / 60;
    seconds %= 60;
    
    return QString("%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'));
}

void MainWindow::loadSong(const QString &filePath)
{
    mediaPlayer->setSource(QUrl::fromLocalFile(filePath));
    
    // Update UI
    QFileInfo fileInfo(filePath);
    songTitleLabel->setText(fileInfo.baseName());
    artistLabel->setText("Loading...");
    albumLabel->setText("Loading...");
    albumArtLabel->setText("Loading...");
    
    // Update playlist selection
    for (int i = 0; i < playlistListWidget->count(); ++i) {
        playlistListWidget->item(i)->setSelected(i == currentIndex);
    }
    
    // Scroll to current item
    if (currentIndex >= 0 && currentIndex < playlistListWidget->count()) {
        playlistListWidget->scrollToItem(playlistListWidget->item(currentIndex));
    }
}

void MainWindow::updatePlaylist()
{
    playlistListWidget->clear();
    
    for (const QString &filePath : currentPlaylist) {
        QFileInfo fileInfo(filePath);
        playlistListWidget->addItem(fileInfo.baseName());
    }
    
    // Update selection
    if (currentIndex >= 0 && currentIndex < playlistListWidget->count()) {
        playlistListWidget->item(currentIndex)->setSelected(true);
    }
}

void MainWindow::shufflePlaylist()
{
    if (currentPlaylist.size() <= 1) return;
    
    // Remember current song
    QString currentSong = currentIndex >= 0 && currentIndex < currentPlaylist.size() 
                        ? currentPlaylist[currentIndex] : QString();
    
    // Shuffle playlist
    QStringList shuffled = currentPlaylist;
    
    // Fisher-Yates shuffle algorithm
    QRandomGenerator *rng = QRandomGenerator::global();
    for (int i = shuffled.size() - 1; i > 0; --i) {
        int j = rng->bounded(i + 1);
        if (i != j) {
            shuffled.swapItemsAt(i, j);
        }
    }
    
    // Update playlist
    currentPlaylist = shuffled;
    updatePlaylist();
    
    // Find new index of current song
    if (!currentSong.isEmpty()) {
        currentIndex = currentPlaylist.indexOf(currentSong);
    }
}
