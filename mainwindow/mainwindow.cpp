#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent),
          ui(new Ui::MainWindow),
          m_configobj_update_flag(false) {
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/ui/resource/icon.ico"));

    ui->config_widget->setColumnWidth(0, 220);

    loadConfigWidgetConnect();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_button_load_clicked() {
    QString file_path = QFileDialog::getOpenFileName(this, tr("Choose Config File"), "", tr("Config Files (*.txt)"));
    if (!file_path.isEmpty()) {
        updateConfigView(file_path);
    }
}

void MainWindow::on_button_update_clicked() {
    m_config.updateConfig();
}

void MainWindow::on_button_save_clicked() {
    QString file_path = QFileDialog::getSaveFileName(this, tr("Save Config File"), "", tr("Config Files (*.txt)"));
    if (!file_path.isEmpty()) {
        m_config.saveConfig(file_path);
    }
}

void MainWindow::loadConfigWidgetConnect() {
    connect(ui->config_widget, &QTreeWidget::itemDoubleClicked, this, [&](QTreeWidgetItem *item, int column) {
        if (column != 1) { // 只允许“值”列可编辑（假设第1列为值）
            ui->config_widget->setEditTriggers(QAbstractItemView::NoEditTriggers); // 临时禁用编辑
        } else {
            ui->config_widget->setEditTriggers(QAbstractItemView::DoubleClicked); // 允许编辑
        }
    });

    connect(ui->config_widget, &QTreeWidget::itemChanged, this, [&](QTreeWidgetItem *item, int column) {
        if (m_configobj_update_flag && column == 1) { // 如果修改的是“值”列
            QString value = item->text(1); // 获取修改后的值（右列）
            updateConfigObj(item, value); // 调用自定义函数，更新底层 JSON 数据
        }
    });
}

void MainWindow::updateConfigView(QString config_path) {
    m_config = JsonConfig(config_path);

    ui->config_widget->clear();

    m_configobj_update_flag = false;
    addJsonDataToTree(m_config.getJsonData(), nullptr, ui->config_widget);
    m_configobj_update_flag = true;
}

void MainWindow::updateConfigObj(QTreeWidgetItem *item, QString value) {
    // 遍历路径，找到对应的 JSON 节点
    QStringList path_list;
    QTreeWidgetItem *current_item = item;
    while (current_item->parent()) {
        path_list.prepend(current_item->text(0));
        current_item = current_item->parent();
    }
    path_list.prepend(current_item->text(0));

    // 遍历 JSON 数据，找到对应的键
    json_t *target = &m_config.getJsonData();
    for (const auto &key : path_list) {
        if (target->is_object() && target->contains(key.toStdString())) {
            target = &(*target)[key.toStdString()];
        } else if (target->is_array()) {
            bool ok = false;
            int index = key.mid(1, key.length() - 2).toInt(&ok); // 提取 [0], [1] 等数组索引
            if (ok && index < target->size()) {
                target = &(*target)[index];
            } else {
                return; // 路径无效，直接退出
            }
        } else {
            return; // 路径无效，直接退出
        }
    }

    // 更新值
    if (target && target->is_string()) {
        *target = value.toStdString();
    } else if (target && (target->is_number() || target->is_boolean())) {
        // 尝试将字符串转换为合适的类型
        if (value == "true" || value == "false") {
            *target = (value == "true");
        } else {
            bool isInt = false;
            bool isDouble = false;

            int intValue = value.toInt(&isInt);
            double doubleValue = value.toDouble(&isDouble);

            if (isInt) {
                *target = intValue; // 更新为整数
            } else if (isDouble) {
                *target = doubleValue; // 更新为浮点数
            } else {
                *target = value.toStdString(); // 如果既不是整数也不是浮点数，按字符串处理
            }
        }
    } else {
        *target = value.toStdString(); // 默认作为字符串处理
    }
}

void MainWindow::addJsonDataToTree(const json_t &json_data,
                                   QTreeWidgetItem *parent_item,
                                   QTreeWidget *tree_widget,
                                   const QString &key_name) {
    if (json_data.is_object()) {
        for (const auto& [key, value] : json_data.items()) {
            QTreeWidgetItem* item = new QTreeWidgetItem();
            item->setText(0, QString::fromStdString(key)); // 左列显示键名
            if (parent_item) {
                parent_item->addChild(item);
            } else if (tree_widget) {
                tree_widget->addTopLevelItem(item);
            }
            addJsonDataToTree(value, item); // 递归处理子节点
        }
    } else if (json_data.is_array()) {
        for (size_t i = 0; i < json_data.size(); ++i) {
            QTreeWidgetItem* item = new QTreeWidgetItem();
            item->setText(0, QString("[%1]").arg(i)); // 左列显示数组索引
            if (parent_item) {
                parent_item->addChild(item);
            }
            addJsonDataToTree(json_data[i], item); // 递归处理数组中的元素
        }
    } else {
        // 基本类型，显示键名和对应值在同一行
        QTreeWidgetItem* item = parent_item;
        if (!parent_item) {
            item = new QTreeWidgetItem();
            if (tree_widget) {
                tree_widget->addTopLevelItem(item);
            }
        }
        if (!key_name.isEmpty()) {
            item->setText(0, key_name); // 左列显示键名
        }
        item->setText(1, QString::fromStdString(json_data.dump())); // 右列显示值
        item->setFlags(item->flags() | Qt::ItemIsEditable); // 设置可编辑
    }
}

