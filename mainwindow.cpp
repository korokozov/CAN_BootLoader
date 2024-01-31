#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QtMath>

int GetRecordFromString(HexRecord &hexrec, std::string hexstr) {
    uint16_t Len = hexstr.length();
    if(Len < 11)
        return 0;
    if(hexstr.substr(0,1) != ":")
        return 0;
    uint16_t reclen = std::stoi(hexstr.substr(1,2), nullptr, 16);
    if(Len < reclen*2 + 11)
        return 0;
    hexrec.RecLen = reclen;
    hexrec.MemOffset = std::stoi(hexstr.substr(3,4), nullptr, 16);
    hexrec.RecType = std::stoi(hexstr.substr(7,2), nullptr, 16);
    for(int i=0; i<hexrec.RecLen; i++)
        hexrec.Data_Or_Info[i] = std::stoi(hexstr.substr(9+2*i,2), nullptr, 16);
    hexrec.crc8 = std::stoi(hexstr.substr(9+2*hexrec.RecLen,2), nullptr, 16);
    return 1;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_loadHexFile(std::vector<HexRecord> records)
{
    if (records.size()==0)
        return;

    std::vector<HexRecord>::iterator iter = records.begin();
    HexRecord tmp_record = *iter;
    int row=0;
    int col = ui->CountBit_comboBox->currentIndex()+2;
    col  = (uint32_t)qPow(2,col);

    ui->tableWidget->setColumnCount(col+2);
    ui->tableWidget->setRowCount(records.size());

    uint32_t address = (tmp_record.Data_Or_Info[0]<<8 + tmp_record.Data_Or_Info[1])<<16;
    ui->Address_lineEdit->setText((QString("0x%1").arg(address,8,16,QLatin1Char('0'))).toUpper());

    ui->tableWidget->setHorizontalHeaderItem(0,new QTableWidgetItem("       Address       "));
    ui->tableWidget->horizontalHeader()->setStyleSheet("QHeaderView::section{background:rgb(240,240,240);}");
    ui->tableWidget->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignCenter);

    QString str("");
    uint16_t step = (uint16_t)16/col;
    str.fill(' ', step*3);
    for (int i=1;i<=col;i++){
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem((str + QString("%1").arg((i-1)*step,0,16,QLatin1Char('0')) + str).toUpper()));
        ui->tableWidget->horizontalHeaderItem(i)->setTextAlignment(Qt::AlignCenter);
    }
    ui->tableWidget->setHorizontalHeaderItem(col+1, new QTableWidgetItem("ASCII"));
    ui->tableWidget->horizontalHeaderItem(col+1)->setTextAlignment(Qt::AlignCenter);

    row = 0;
    foreach (tmp_record, records) {
        if (tmp_record.RecType == 0){
            ui->tableWidget->setItem(row,0, new QTableWidgetItem((QString("0x%1").arg(address+tmp_record.MemOffset,8,16,QLatin1Char('0'))).toUpper()));
            ui->tableWidget->item(row,0)->setBackgroundColor(QColor::fromRgb(235,235,235,255));

            uint32_t tmp=0;
            str="";
            for (int i=0;i<tmp_record.RecLen;i+=step){
                tmp = 0;
                for (int j=0;j<step;j++){
                    tmp = tmp + (tmp_record.Data_Or_Info[i+j]<<(j*8));
                    str=str+(char)tmp_record.Data_Or_Info[i+j];
                }
                ui->tableWidget->setItem(row,i/step+1, new QTableWidgetItem((QString("%1").arg(tmp,(int)step*2,16,QLatin1Char('0'))).toUpper()));
                ui->tableWidget->item(row,i/step+1)->setBackgroundColor(QColor::fromRgb(225,227,232,50*(row%2)));
            }
            for (int i=tmp_record.RecLen;i<MAX_REC_DATA_LENGTH;i++){
                ui->tableWidget->setItem(row,i/step+1, new QTableWidgetItem(""));
                ui->tableWidget->item(row,i/step+1)->setBackgroundColor(QColor::fromRgb(225,227,232,50*(row%2)));
            }
            ui->tableWidget->setItem(row,col+1, new QTableWidgetItem(str));
            ui->tableWidget->item(row,col+1)->setBackgroundColor(QColor::fromRgb(225,227,232,60*(row%2)));
            row++;
        }
    }
    ui->tableWidget->setRowCount(row);
    ui->tableWidget->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

}



void MainWindow::on_CountBit_comboBox_currentIndexChanged(int index)
{
    on_loadHexFile(records);
}

void MainWindow::on_loadHexFile_Button_clicked()
{
    fileName = QFileDialog::getOpenFileName(this,
                                            "Загрузка файла",
                                            "",
                                            "hex-файл (*.hex)");
    if(fileName == "")
        return;

    ui->textBrowser->append("Open file: " + fileName);


    std::vector<std::string> lines;
    std::string line;
    std::ifstream in(fileName.toStdString());
    if (in.is_open())
    {
        while (std::getline(in, line))
        {
            std::cout << line << std::endl;
            lines.push_back(line);
        }
    }
    in.close();

    records.clear();
    HexRecord tmp_record;
    for(int i=0; i<lines.size(); i++) {
        if(GetRecordFromString(tmp_record, lines.at(i))) {
            records.push_back(tmp_record);
        }
        else {
            records.clear();
            break;
        }
    }

    on_loadHexFile(records);
}
