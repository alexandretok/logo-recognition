#include "settings.h"
#include "ui_settings.h"

Settings::Settings(QWidget *parent) : QDialog(parent), ui(new Ui::Settings) {
    ui->setupUi(this);
    setFixedSize(this->geometry().size());
}

Settings::~Settings() {
    delete ui;
}
