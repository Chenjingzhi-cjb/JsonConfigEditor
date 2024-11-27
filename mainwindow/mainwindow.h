#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QFileDialog>

#include "json_config.hpp"


QT_BEGIN_NAMESPACE
    namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_button_load_clicked();

    void on_button_update_clicked();

    void on_button_save_clicked();

private:
    void loadConfigWidgetConnect();

    void updateConfigView(QString config_path);

    void updateConfigObj(QTreeWidgetItem *item, QString value);

    void addJsonDataToTree(const json_t &json_data,
                           QTreeWidgetItem *parent_item = nullptr,
                           QTreeWidget *tree_widget = nullptr,
                           const QString &key_name = "");

private:
    Ui::MainWindow *ui;

    JsonConfig m_config;
    bool m_configobj_update_flag;
};


#endif // MAINWINDOW_H
