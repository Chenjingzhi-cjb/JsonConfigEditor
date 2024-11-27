#ifndef JSON_CONFIG_HPP
#define JSON_CONFIG_HPP

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QString>

#include <iostream>
#include <string>
#include <vector>

#include "json.hpp"


using json_t = nlohmann::json;


class JsonConfig {
public:
    explicit JsonConfig(QString config_path)
            : m_config_path(std::move(config_path)) {
        loadConfig();
    }

    JsonConfig() = default;

    ~JsonConfig() = default;

public:
    json_t &getJsonData() {
        return m_json_data;
    }

    void loadConfig() {
        if (m_config_path.isEmpty()) return;

        QFile config_file(m_config_path);
        if (config_file.open(QIODevice::ReadOnly)) {
            QTextStream in(&config_file);
            std::string config_info = in.readAll().toStdString();
            config_file.close();

            try {
                m_json_data = json_t::parse(config_info);
            } catch (json_t::exception &e) {
                std::cout << "Failed to parse the config file: " << e.what() << std::endl;
            }
        } else {
            std::cout << "Failed to open the config file." << std::endl;
        }
    }

    void updateConfig() {
        saveConfig(m_config_path);
    }

    void saveConfig(const QString &config_path) {
        if (m_config_path.isEmpty()) return;

        QFile config_file(config_path);
        if (config_file.open(QIODevice::WriteOnly)) {
            QTextStream out(&config_file);

            try {
                out << QString::fromStdString(m_json_data.dump(4));
                config_file.close();
            } catch (json_t::exception &e) {
                std::cout << "Failed to serialize the config data: " << e.what() << std::endl;
            }
        } else {
            std::cout << "Failed to open the config file for writing." << std::endl;
        }
    }

private:
    QString m_config_path;

    json_t m_json_data;
};


#endif // JSON_CONFIG_HPP
