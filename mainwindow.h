#include <QMainWindow>
#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QImage>
#include <QMouseEvent>
#include <QSlider>
#include <QAbstractSlider>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QDir>
#include <QFile>
#include <array>
#include <QVector>
#include <QDataStream>
#include <QDebug>
#include <QTextStream>
#include <QTimer>
#include <QFile>
#include <QByteArray>
#include <random>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    QImage *img1;
    QImage *img;

    int szer;
    int wys;
    int poczX;
    int poczY;
    int startX;
    int startY;

    std::array<uint8_t, 4096> memory; // 4kb ram memory
    bool display[32][64]; // display 64x32p
    uint16_t programCounter; //PC
    uint16_t indexRegister; //I
    uint8_t registers[16]; //registers from 0(V0) to F(VF)

    bool keys[16];

    std::array<uint16_t, 16> stack;
    uint8_t stackPointer;

    QTimer cpuTimer; //cpu frequency
    QTimer timerTimer; //timer setting delay and sound timer

    uint8_t delayTimer;
    uint8_t soundTimer;

    bool super; //original cosmac vip or super-chip/chip-48
    bool amiga; //original cosmac vip or amiga chip-8 interpreter

    std::mt19937 rng;
    std::uniform_int_distribution<int> dist{0, 255};

    const std::array<uint8_t, 80> fontset= { //font
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    void keyPressEvent(QKeyEvent *event) override; // listening to keys
    void keyReleaseEvent(QKeyEvent *event) override;

    std::unordered_map<int, uint8_t> keyMap = { // keyboard
        { Qt::Key_1, 0x1 },
        { Qt::Key_2, 0x2 },
        { Qt::Key_3, 0x3 },
        { Qt::Key_4, 0xC },

        { Qt::Key_Q, 0x4 },
        { Qt::Key_W, 0x5 },
        { Qt::Key_E, 0x6 },
        { Qt::Key_R, 0xD },

        { Qt::Key_A, 0x7 },
        { Qt::Key_S, 0x8 },
        { Qt::Key_D, 0x9 },
        { Qt::Key_F, 0xE },

        { Qt::Key_Z, 0xA },
        { Qt::Key_X, 0x0 },
        { Qt::Key_C, 0xB },
        { Qt::Key_V, 0xF }
    };


    QString file;
    QString folderPath;

    void czysc();
    void openFile(QString path);
    void clearScreen();
    void drawScreen();

    void paintEvent(QPaintEvent*) override;

private slots:
    void cycle();
    void updateTimers();

};
