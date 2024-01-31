#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <iostream>
#include <fstream>
#include <string>

#define MAX_REC_DATA_LENGTH     16

typedef struct {
    uint8_t RecLen;
    uint16_t MemOffset;
    uint8_t RecType;
    uint8_t Data_Or_Info[MAX_REC_DATA_LENGTH];
    uint16_t crc8;
}HexRecord;

int GetRecordFromString(HexRecord &hexrec, std::string hexstr);

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_loadHexFile(std::vector<HexRecord> records);

    void on_CountBit_comboBox_currentIndexChanged(int index);

    void on_loadHexFile_Button_clicked();

private:
    Ui::MainWindow *ui;
    QString fileName="";
    std::vector<HexRecord> records;
};

#endif // MAINWINDOW_H
