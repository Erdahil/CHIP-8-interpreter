#include "mainwindow.h"

#include <iostream>
#include <algorithm>

#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent): QWidget(parent)
{
    setWindowTitle("CHIP-8");
    resize(640,320);

    szer = 640;
    wys = 320;
    poczX = 0;
    poczY = 0;


    cpuTimer.setInterval(1000 / 700);//should be probably faster for some modern games
    timerTimer.setInterval(1000 / 60);

    //connect(cpuTimer, &QTimer::timeout, this, &MainWindow::cycle());//wtf does that not work
    connect(&cpuTimer, SIGNAL(timeout()), this, SLOT(cycle()));
    connect(&timerTimer, SIGNAL(timeout()), this, SLOT(updateTimers()));

    for(int i=0; i<4096; i++)
    {
        memory[i]=0;
    }

    for(int i=0; i<16; i++)
    {
        registers[i]=0;
    }

    for(int i=0; i<16; i++)
    {
        stack[i]=0;
    }

    for(int i=0; i<32; i++)
    {
        for(int j=0; j<64; j++)
        {
            display[i][j]=false;
        }
    }

    std::copy(fontset.begin(), fontset.end(), memory.begin() + 0x50);

    stackPointer=0;

    super=false;// hardware settings here
    amiga=false;


    img1 = new QImage(szer,wys,QImage::Format_RGB32);

    std::random_device rd;
    rng.seed(rd());

    czysc();

    openFile("C:/Users/pawel/OneDrive/Dokumenty/QtProjects/CHIP-8/programs/snake.ch8");
    //openFile("C:/Users/pawel/OneDrive/Dokumenty/QtProjects/CHIP-8/programs/test_opcode.ch8");
    //openFile("C:/Users/pawel/OneDrive/Dokumenty/QtProjects/CHIP-8/programs/IBM_Logo.ch8");



}

void MainWindow::cycle()
{
    //fetch
    uint16_t opcode = (memory[programCounter] << 8) | memory[programCounter + 1]; //moves to the left to join the second byte of the instruction - joining two bytes into one instruction
    programCounter += 2;

    //decode
    uint8_t W = (opcode & 0xF000) >> 12; //first nibble //wth is a 'nibble' - the first one states which kind of instruction
    uint8_t X = (opcode & 0x0F00) >> 8; //second nibble - for looking up registers
    uint8_t Y = (opcode & 0x00F0) >> 4; //third nibble - also for registers
    uint8_t N = opcode & 0x000F; //fourth nibble - just 4bit number

    uint8_t NN = opcode & 0x00FF; //3rd and 4th nibble - 8bit 'instant' number (??)
    uint16_t NNN = opcode & 0x0FFF; //2nd, 3rd and 4th nibble - memory address

    uint8_t x,y; //coords for drawing

    bool keyFound; //for finding pressed key

    switch(W)
    {
        case 0x0:
            if(opcode==0x00E0)
            {
                clearScreen();
            }
            else if(opcode==0x00EE)
            {
                --stackPointer;
                programCounter=stack[stackPointer];//it could be safeguarded but who cares
            }
            else
            {
                //0NNN omitting
            }
            break;
        case 0x1:
            programCounter=NNN;
            break;
        case 0x2:
            stack[stackPointer] = programCounter;
            stackPointer++;
            programCounter=NNN;
            break;
        case 0x3:
            if(registers[X]==NN)
            {
                programCounter+=2;
            }
            break;
        case 0x4:
            if(registers[X]!=NN)
            {
                programCounter+=2;
            }
            break;
        case 0x5:
            if(N==0 /*idk if that needs to be here*/&& registers[X]==registers[Y])
            {
                programCounter+=2;
            }
            break;
        case 0x6:
            registers[X]=NN;
            break;
        case 0x7:
            registers[X]+=NN;
            break;
        case 0x8:
            switch (N)
            {
                case 0x0:
                    registers[X]=registers[Y];
                    break;
                case 0x1:
                    registers[X]=registers[X] | registers[Y];
                    break;
                case 0x2:
                    registers[X]=registers[X] & registers[Y];
                    break;
                case 0x3:
                    registers[X]=registers[X] ^ registers[Y];
                    break;
                case 0x4:
                    if(static_cast<uint16_t>(registers[X]+registers[Y])>255) //does that even work?
                    {
                        registers[15]=1;
                    }
                    else
                    {
                        registers[15]=0;//idk if that needs to be here
                    }
                    registers[X]=registers[X] + registers[Y];
                    break;
                case 0x5:
                    if(registers[X]>=registers[Y])
                    {
                        registers[15]=1;
                    }
                    else
                    {
                        registers[15]=0;
                    }
                    registers[X]=registers[X] - registers[Y];
                    break;
                case 0x6:
                    if(!super)
                    {
                        registers[X]=registers[Y];
                    }
                    registers[15]=registers[X]&0x01;
                    registers[X]>>=1;
                    break;
                case 0x7:
                    if(registers[Y]>=registers[X])
                    {
                        registers[15]=1;
                    }
                    else
                    {
                        registers[15]=0;
                    }
                    registers[X]=registers[Y] - registers[X];
                    break;
                case 0xE:
                    if(!super)
                    {
                        registers[X]=registers[Y];
                    }
                    registers[15]=(registers[X]&0x80)>>7;
                    registers[X]<<=1;
                    break;
            }
            break;
        case 0x9:
            if(registers[X]!=registers[Y])
            {
                programCounter+=2;
            }
            break;
        case 0xA:
            indexRegister=NNN;
            break;
        case 0xB:
            if(!super)
            {
                programCounter=NNN+registers[0];
            }
            else
            {
                programCounter=NNN+registers[X];
            }
            break;
        case 0xC:
            registers[X]=static_cast<uint8_t>(dist(rng))&NN;
            break;
        case 0xD:
            y=registers[Y]%32;
            registers[15]=0;

            for(int i=0; i<N; i++)
            {
                x=registers[X]%64;
                for(int j=0; j<8; j++)
                {
                    if(memory[indexRegister+i]&(0x80>>j) && display[y][x])
                    {
                        display[y][x]=!display[y][x];
                        registers[15]=1;
                    }
                    else if(memory[indexRegister+i]&(0x80>>j) && !display[y][x])
                    {
                        display[y][x]=!display[y][x];
                    }

                    if(x>=63)
                    {
                        break;
                    }

                    x++;
                }
                y++;

                if(y>=32)
                {
                    break;
                }
            }

            drawScreen();
            break;
        case 0xE:
            switch (NN)
            {
                case 0x9E:
                    if(keys[registers[X]])
                    {
                        programCounter+=2;
                    }
                    break;
                case 0xA1:
                    if(!keys[registers[X]])
                    {
                        programCounter+=2;
                    }
                    break;
            }

            break;
        case 0xF:
            switch (NN)
            {
                case 0x07:
                    registers[X]=delayTimer;
                    break;
                case 0x15:
                    delayTimer=registers[X];
                    break;
                case 0x18:
                    soundTimer=registers[X];
                    break;
                case 0x1E:
                    if(amiga)
                    {
                        registers[15]=static_cast<uint16_t>(indexRegister+=registers[X])>0x0FFF ? 1 : 0;
                    }
                    indexRegister+=registers[X];
                    break;
                case 0x0A:
                    keyFound=false;
                    for(int i=0; i<16; i++)
                    {
                        if(keys[i]==true)
                        {
                            registers[X]=i;
                            keyFound=true;
                            break;
                        }
                    }

                    if(!keyFound)
                    {
                        programCounter-=2;
                    }
                    break;
                case 0x29:
                    indexRegister=(registers[X]&0x0F)*5 + 0x50; //*5 because its the font size
                    break;
                case 0x33:
                    memory[indexRegister]=registers[X]/100;
                    memory[indexRegister+1]=registers[X]/10%10;
                    memory[indexRegister+2]=registers[X]%10;
                    break;
                case 0x55:
                    for(int i=0; i<=X; i++)
                    {
                        memory[indexRegister+i]=registers[i];
                    }
                    if(!super)
                    {
                        indexRegister+=X+1;
                    }
                    break;
                case 0x65:
                    for(int i=0; i<=X; i++)
                    {
                        registers[i]=memory[indexRegister+i];
                    }
                    if(!super)
                    {
                        indexRegister+=X+1;
                    }
                    break;
            }

            break;
    }

}

void MainWindow::updateTimers()
{
    if(delayTimer>0)
    {
        delayTimer--;
    }

    if(soundTimer>0)
    {
        soundTimer--;
    }

}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    auto it = keyMap.find(event->key());

    if(it != keyMap.end())
    {
        uint8_t chip8Key = it->second;

        qDebug() << "CHIP-8 key pressed:" << Qt::hex << chip8Key;
        keys[chip8Key] = true;
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    auto it = keyMap.find(event->key());

    if(it != keyMap.end())
    {
        uint8_t chip8Key = it->second;

        qDebug() << "CHIP-8 key released:" << Qt::hex << chip8Key;
        keys[chip8Key] = false;
    }
}

void MainWindow::clearScreen()
{
    for(int i=0; i<32; i++)
    {
        for(int j=0; j<64; j++)
        {
            display[i][j]=false;
        }
    }
    czysc();
}

void MainWindow::drawScreen()
{
    unsigned char *ptr;
    ptr = img1->bits();

    int i,j;

    for(i=0; i<wys; i++)
    {
        for(j=0; j<szer; j++)
        {
            if(display[i/10][j/10])
            {
                ptr[szer*4*i + 4*j] = 255;
                ptr[szer*4*i + 4*j + 1] = 255;
                ptr[szer*4*i + 4*j + 2] = 255;
            }
            else
            {
                ptr[szer*4*i + 4*j] = 0;
                ptr[szer*4*i + 4*j + 1] = 0;
                ptr[szer*4*i + 4*j + 2] = 0;
            }
        }
    }
    update();
}

void MainWindow::czysc()
{
    unsigned char *ptr;
    ptr = img1->bits();

    int i,j;

    for(i=0; i<wys; i++)
    {
        for(j=0; j<szer; j++)
        {
            ptr[szer*4*i + 4*j] = 0;
            ptr[szer*4*i + 4*j + 1] = 0;
            ptr[szer*4*i + 4*j + 2] = 0;
        }
    }
    update();
}


void MainWindow::openFile(QString path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Nie udało się otworzyć pliku ROM:" << path;
    }
    QByteArray romData=file.readAll();
    file.close();

    for(int i=0; i<romData.size(); i++)
    {
        memory[0x200+i]=static_cast<uint8_t>(romData[i]);
    }

    programCounter=0x200;

    std::cout<<"Loaded"<<std::endl;
    cpuTimer.start();
    timerTimer.start();
}


void MainWindow::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    p.drawImage(poczX, poczY, *img1);
}


MainWindow::~MainWindow()
{
    delete img;
}
