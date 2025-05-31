#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QListWidget>
#include <QSlider>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QFileSystemModel>
#include <QTreeView>
#include <QTabWidget>
#include <QLineEdit>
#include <QStandardItemModel>
#include <QTableView>
#include <QMediaMetaData>
#include <QSettings>
#include <QProgressDialog>
#include <QDirIterator>
#include <QEventLoop>
#include <QTimer>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openFile();
    void playPause();
    void stop();
    void next();
    void previous();
    void seekChanged(int position);
    void updatePosition(qint64 position);
    void updateDuration(qint64 duration);
    void updateMetadata();
    void setVolume(int volume);
    void toggleMute();
    void toggleRepeat();
    void toggleShuffle();
    void createPlaylist();
    void loadPlaylist();
    void savePlaylist();
    void addToPlaylist();
    void removeFromPlaylist();
    void playlistItemDoubleClicked(QListWidgetItem *item);
    void searchLibrary();
    void scanLibrary();
    void editMetadata();
    void applyEqualizer(int band, int value);
    void saveEqualizerPreset();
    void loadEqualizerPreset();
    void setSleepTimer();

private:
    void setupUi();
    void setupConnections();
    void setupMenus();
    void loadSettings();
    void saveSettings();
    void updatePlaybackInfo();
    QString formatTime(qint64 ms);
    void loadSong(const QString &filePath);
    void updatePlaylist();
    void shufflePlaylist();
    
    // Core media components
    QMediaPlayer *mediaPlayer;
    QAudioOutput *audioOutput;
    
    // UI components
    QTabWidget *tabWidget;
    
    // Now Playing tab
    QWidget *nowPlayingTab;
    QLabel *albumArtLabel;
    QLabel *songTitleLabel;
    QLabel *artistLabel;
    QLabel *albumLabel;
    QSlider *seekSlider;
    QLabel *currentTimeLabel;
    QLabel *totalTimeLabel;
    QPushButton *playPauseButton;
    QPushButton *stopButton;
    QPushButton *previousButton;
    QPushButton *nextButton;
    QPushButton *shuffleButton;
    QPushButton *repeatButton;
    QSlider *volumeSlider;
    QPushButton *muteButton;
    
    // Library tab
    QWidget *libraryTab;
    QLineEdit *searchBox;
    QPushButton *searchButton;
    QTableView *libraryTableView;
    QStandardItemModel *libraryModel;
    
    // Playlists tab
    QWidget *playlistsTab;
    QListWidget *playlistListWidget;
    QPushButton *addToPlaylistButton;
    QPushButton *removeFromPlaylistButton;
    QPushButton *createPlaylistButton;
    QPushButton *loadPlaylistButton;
    QPushButton *savePlaylistButton;
    
    // Equalizer tab
    QWidget *equalizerTab;
    QVector<QSlider*> equalizerSliders;
    QComboBox *equalizerPresets;
    QPushButton *saveEqualizerButton;
    
    // File browser tab
    QWidget *fileBrowserTab;
    QFileSystemModel *fileSystemModel;
    QTreeView *fileSystemView;
    
    // State variables
    bool isPlaying;
    bool isMuted;
    bool isShuffled;
    int repeatMode; // 0: no repeat, 1: repeat all, 2: repeat one
    QStringList currentPlaylist;
    int currentIndex;
    QMap<QString, QVariant> currentMetadata;
    QSettings settings;
};

#endif // MAINWINDOW_H
