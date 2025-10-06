#include <QWidget>
#include <QPainter>
#include <QVBoxLayout>
#include <QRandomGenerator>
#include <QShortcut>
#include <QPalette>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

using namespace Qt::StringLiterals;

class Utils : public QObject {
    Q_OBJECT

public:
    inline explicit Utils(QObject* parent = nullptr) : QObject{ parent } {}
    inline ~Utils() override = default;

public Q_SLOTS:
    [[nodiscard]] static inline QColor generateRandomColor() {
        const auto& randomChannel{ [](){ return QRandomGenerator::global()->bounded(256); } };
        return QColor::fromRgb(randomChannel(), randomChannel(), randomChannel());
    }

    [[nodiscard]] static inline bool isColorLight(const QColor& color) {
        Q_ASSERT(color.isValid());
        const auto& toLinear{ [](const qreal value) {
            Q_ASSERT(qFuzzyIsNull(value) || value > qreal(0));
            Q_ASSERT(qFuzzyCompare(value, qreal(1)) || value < qreal(1));
            static constexpr const qreal magic{ 0.03928 };
            return (qFuzzyCompare(value, magic) || value < magic) ? (value / 12.92) : qPow((value + 0.055) / 1.055, 2.4);
        } };
        const auto linearR{ toLinear(color.redF()) };
        const auto linearG{ toLinear(color.greenF()) };
        const auto linearB{ toLinear(color.blueF()) };
        const auto luminance{ 0.2126 * linearR + 0.7152 * linearG + 0.0722 * linearB };
        return luminance > 0.5;
    }
};

static inline void paintReveal(const QWidget* widget, QPainter* painter, const QRect& rect) {
    Q_ASSERT(widget);
    Q_ASSERT(painter);

    QColor light{ painter->pen().color() }; // widget->palette().light().color();
    light.setAlpha(128);
    const QPoint mousePos{ widget->mapFromGlobal(QCursor::pos(widget->screen())) };
    const bool pressed{ widget->rect().contains(mousePos) && (QGuiApplication::mouseButtons() & Qt::LeftButton) };

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    if (Utils::isColorLight(light)) {
        painter->setOpacity(pressed ? 0.7 : 0.5);
    } else {
        painter->setOpacity(pressed ? 0.5 : 0.3);
    }

    const auto radius{ qMax(rect.width(), rect.height()) };
    static constexpr const int border{ 1 };

    QRadialGradient gradient(mousePos, radius, mousePos);

    gradient.setColorAt(qreal(0), light);
    gradient.setColorAt(qreal(1), QColorConstants::Transparent);

    painter->fillRect(rect.adjusted(border, border, -border, -border), gradient);
    painter->restore();
}

static inline void paintRevealFrame(const QWidget* widget, QPainter* painter, const QRect& rect) {
    Q_ASSERT(widget);
    Q_ASSERT(painter);

    QColor light = painter->pen().color(); // widget->palette().light().color();
    //light.setAlpha(240);
    const QPoint mousePos{ widget->mapFromGlobal(QCursor::pos(widget->screen())) };
    const bool pressed{ widget->rect().contains(mousePos) && (QGuiApplication::mouseButtons() & Qt::LeftButton) };

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setOpacity(pressed ? 0.8 : 0.5);

    const auto radius{ qMin(rect.width(), rect.height()) };
    static constexpr const int border{ 2 };

    QRadialGradient gradient(mousePos, radius, mousePos);

    gradient.setColorAt(qreal(0), light);
    gradient.setColorAt(qreal(1), QColorConstants::Transparent);

    QRegion frameRegion{ rect };
    frameRegion -= rect.adjusted(border, border, -border, -border);
    painter->setClipRegion(frameRegion);
    //painter->setOpacity(painter->opacity() * 0.9);
    painter->fillRect(rect, gradient);
    painter->restore();
}

class RevealWidget final : public QWidget {
    Q_OBJECT

public:
    inline explicit RevealWidget(QWidget* parent = nullptr) : QWidget{ parent } {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setMouseTracking(true);
        {
            QPalette pal{ palette() };
            pal.setColor(QPalette::Light, QColorConstants::White);
            setPalette(pal);
        }
        new QShortcut(QKeySequence::Refresh, this, [this](){ updateColor(); });
        updateColor();
    }

    inline ~RevealWidget() override = default;

    [[nodiscard]] inline const QColor& color() const {
        return m_color;
    }

public Q_SLOTS:
    inline void updateColor() {
        m_color = std::move(Utils::generateRandomColor());
        update();
        Q_EMIT colorChanged();
    }

Q_SIGNALS:
    void colorChanged();

protected:
    inline void mouseMoveEvent(QMouseEvent* event) override {
        QWidget::mouseMoveEvent(event);
        update();
    }

    inline void mousePressEvent(QMouseEvent* event) override {
        QWidget::mousePressEvent(event);
        update();
    }

    inline void mouseReleaseEvent(QMouseEvent* event) override {
        QWidget::mouseReleaseEvent(event);
        update();
    }

    inline void enterEvent(QEnterEvent* event) override {
        QWidget::enterEvent(event);
        update();
    }

    inline void leaveEvent(QEvent* event) override {
        QWidget::leaveEvent(event);
        update();
    }

    inline void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        const QRect r{ rect() };
        painter.fillRect(r, m_color);
        painter.setPen(Utils::isColorLight(m_color) ? QColorConstants::Black : QColorConstants::White);
        if (underMouse()) {
            paintReveal(this, &painter, r);
            paintRevealFrame(this, &painter, r);
        }
    }

private:
    QColor m_color{};
};

class Window final : public QWidget {
    Q_OBJECT

public:
    inline explicit Window(QWidget* parent = nullptr) : QWidget{ parent } {
        setWindowTitle(tr("Reveal Demo [Qt Widgets Edition]"));
        m_widget = new RevealWidget(this);
        connect(m_widget, &RevealWidget::colorChanged, this, [this](){ update(); });
        auto l{ new QVBoxLayout(this) };
        l->setContentsMargins(50, 50, 50, 50);
        l->addWidget(m_widget);
    }

    inline ~Window() override = default;

    inline QSize sizeHint() const override {
        return { 600, 600 };
    }

protected:
    inline void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.fillRect(rect(), Utils::isColorLight(m_widget->color()) ? QColorConstants::Black : QColorConstants::White);
    }

private:
    RevealWidget* m_widget{ nullptr };
};

int main(int argc, char *argv[]) {
    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    QApplication application(argc, argv);

    Window window{};
    window.show();

    Utils utils{};
    QQmlApplicationEngine engine{};
    engine.rootContext()->setContextProperty(u"Utils"_s, QVariant::fromValue(&utils));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &application,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("Demo", "Main");

    return application.exec();
}

#include "main.moc"
