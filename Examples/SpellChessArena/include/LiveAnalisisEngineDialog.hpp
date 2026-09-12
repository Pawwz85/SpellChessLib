#pragma once

#include <cstdint>
#include <optional>

#include <QDialog>

class QComboBox;

class LiveAnalisisEngineDialog : public QDialog {
    Q_OBJECT

    QComboBox *engineSelector;
    std::optional<uint32_t> selectedEngineId;

public:
    explicit LiveAnalisisEngineDialog(QWidget *parent = nullptr);

    std::optional<uint32_t> getSelectedEngineId() const noexcept;
};
