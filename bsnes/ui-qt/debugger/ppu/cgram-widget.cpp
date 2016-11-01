#include "cgram-widget.moc"

CgramWidget::CgramWidget() {
  image = new QImage(16, 16, QImage::Format_RGB32);
  image->fill(0x000000);

  selected = -1;
  setScale(16);
  setPaletteBpp(0);
}

void CgramWidget::setScale(unsigned s) {
  scale = s;

  setFixedSize(16 * scale, 16 * scale);
}

void CgramWidget::setPaletteBpp(unsigned bpp) {
  if(bpp > 8) bpp = 0;

  unsigned nColors = 1 << bpp;

  selectedMask = 0xff - nColors + 1;
  selectedWidth = (nColors - 1) % 16 + 1;
  selectedHeight = (nColors - 1) / 16 + 1;

  selectedChanged();

  update();
}

bool CgramWidget::hasSelected() const {
  return selected >= 0 && selected < 256;
}

unsigned CgramWidget::selectedColor() const {
  return selected & 0xff;
}

unsigned CgramWidget::selectedPalette() const {
  return selected & selectedMask;
}

void CgramWidget::selectNone() {
  setSelected(-1);
}

void CgramWidget::setSelected(int s) {
  if (s != selected) {
    if (s < 0 || s > 255) s = -1;

    selected = s;
    selectedChanged();
  }
}

void CgramWidget::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  painter.setRenderHints(0);
  painter.scale(scale, scale);
  painter.drawImage(0, 0, *image);

  if(selected >= 0 && selected < 256) {
    const static QPen white(Qt::white, 1, Qt::SolidLine);
    const static QPen black(Qt::black, 1, Qt::DashLine);

    painter.resetTransform();

    int s = selected & selectedMask;
    int x = (s % 16) * scale;
    int y = (s / 16) * scale;
    int w = selectedWidth * scale - 1;
    int h = selectedHeight * scale - 1;

    painter.setPen(white);
    painter.drawRect(x, y, w, h);

    painter.setPen(black);
    painter.drawRect(x, y, w, h);
  }
}

void CgramWidget::mousePressEvent(QMouseEvent *event) {
  int x = event->x() / scale;
  int y = event->y() / scale;

  if (x >= 0 && x < 16 && y >= 0 && y < 16) {
    setSelected(y * 16 + x);
  } else {
    setSelected(-1);
  }
}

void CgramWidget::refresh() {
  if(SNES::cartridge.loaded() == false) {
    setSelected(-1);
    image->fill(0x000000);
    update();
    return;
  }

  uint32_t *buffer = (uint32_t*)image->bits();

  for(unsigned i = 0; i < 256; i++) {
    buffer[i] = rgbFromCgram(i);
  }

  update();
}

uint32_t rgbFromCgram(unsigned i) {
  uint16_t color = SNES::memory::cgram[i * 2 + 0];
  color |= SNES::memory::cgram[i * 2 + 1] << 8;

  uint8_t r = (color >>  0) & 31;
  uint8_t g = (color >>  5) & 31;
  uint8_t b = (color >> 10) & 31;

  r = (r << 3) | (r >> 2);
  g = (g << 3) | (g >> 2);
  b = (b << 3) | (b >> 2);

  return (r << 16) | (g << 8) | (b << 0);
}
