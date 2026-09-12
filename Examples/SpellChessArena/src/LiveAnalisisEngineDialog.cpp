#include <LiveAnalisisEngineDialog.hpp>

#include <EngineRepository.hpp>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

LiveAnalisisEngineDialog::LiveAnalisisEngineDialog(QWidget *parent) : QDialog(parent), engineSelector(new QComboBox(this)) {
    setWindowTitle(tr("Select live analysis engine"));
    setModal(true);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Select the engine used for continuous board analysis:"), this));
    layout->addWidget(engineSelector);

    const auto &engines = getEnginePlayerRepositoryInstance().getAll();
    for (const auto &engine : engines) {
        const auto name = engine.name.empty()
            ? QString::fromStdString(engine.pathToEngine)
            : QString::fromStdString(engine.name);
        engineSelector->addItem(name, QVariant::fromValue(engine.entry_id));
    }

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setEnabled(!engines.isEmpty());
    layout->addWidget(buttons);

    connect(engineSelector, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index < 0) {
            selectedEngineId.reset();
            return;
        }

        selectedEngineId = engineSelector->itemData(index).toUInt();
    });
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    if (!engines.isEmpty()) {
        engineSelector->setCurrentIndex(0);
        selectedEngineId = engines.front().entry_id;
    }
}

std::optional<uint32_t> LiveAnalisisEngineDialog::getSelectedEngineId() const noexcept {
    return selectedEngineId;
}

#include <moc_LiveAnalisisEngineDialog.cpp>