#include "app.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>

namespace mo {
namespace {
constexpr int CanvasWidth = 512, CanvasHeight = 384;
constexpr std::size_t MaxParticles = 64;
struct Particle { float x, y, vx, vy, age, life, scale; };
std::array<Particle, MaxParticles> particles{};
std::size_t count{};
HWND trailWindow{};
HDC canvasDC{};
HBITMAP canvasBitmap{};
HGDIOBJ previousBitmap{};
std::uint32_t* pixels{};
bool initialized = false, classRegistered = false;
POINT previousCursor{};
ULONGLONG previousTick{};
float distanceCredit{};
int drawWidth{}, drawHeight{};
std::uint32_t randomState = 0xC31D6A89;

float randomUnit() {
    randomState ^= randomState << 13;
    randomState ^= randomState >> 17;
    randomState ^= randomState << 5;
    return (randomState & 0xFFFF) / 65535.f;
}

void releaseCanvas() {
    if (trailWindow) { DestroyWindow(trailWindow); trailWindow = nullptr; }
    if (canvasDC) {
        if (previousBitmap) SelectObject(canvasDC, previousBitmap);
        if (canvasBitmap) DeleteObject(canvasBitmap);
        DeleteDC(canvasDC);
    }
    canvasDC = nullptr; canvasBitmap = nullptr; previousBitmap = nullptr; pixels = nullptr;
}

bool ensureCanvas() {
    if (trailWindow) return true;
    if (!classRegistered) {
        WNDCLASSW cls{};
        cls.hInstance = instance; cls.lpfnWndProc = DefWindowProcW;
        cls.lpszClassName = L"MouseOptionsMeteorOverlay";
        if (!RegisterClassW(&cls) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) return false;
        classRegistered = true;
    }
    canvasDC = CreateCompatibleDC(nullptr);
    if (!canvasDC) return false;
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = CanvasWidth; info.bmiHeader.biHeight = -CanvasHeight;
    info.bmiHeader.biPlanes = 1; info.bmiHeader.biBitCount = 32; info.bmiHeader.biCompression = BI_RGB;
    void* storage{};
    canvasBitmap = CreateDIBSection(canvasDC, &info, DIB_RGB_COLORS, &storage, nullptr, 0);
    if (!canvasBitmap || !storage) { releaseCanvas(); return false; }
    pixels = static_cast<std::uint32_t*>(storage);
    previousBitmap = SelectObject(canvasDC, canvasBitmap);
    trailWindow = CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        L"MouseOptionsMeteorOverlay", L"Mouse Options Meteors", WS_POPUP,
        0, 0, 1, 1, nullptr, nullptr, instance, nullptr);
    if (!trailWindow) { releaseCanvas(); return false; }
    return true;
}

void over(int x, int y, unsigned color, float opacity) {
    if (x < 0 || x >= drawWidth || y < 0 || y >= drawHeight) return;
    const unsigned alpha = static_cast<unsigned>(std::clamp(opacity * 255.f, 0.f, 255.f));
    if (!alpha) return;
    auto& destination = pixels[y * CanvasWidth + x];
    const unsigned inverse = 255 - alpha;
    const unsigned a = alpha + (destination >> 24) * inverse / 255;
    const unsigned r = ((color >> 16) & 255) * alpha / 255 + ((destination >> 16) & 255) * inverse / 255;
    const unsigned g = ((color >> 8) & 255) * alpha / 255 + ((destination >> 8) & 255) * inverse / 255;
    const unsigned b = (color & 255) * alpha / 255 + (destination & 255) * inverse / 255;
    destination = (a << 24) | (r << 16) | (g << 8) | b;
}

void dot(float x, float y, float radius, float alpha) {
    const int left = static_cast<int>(std::floor(x - radius - 1));
    const int top = static_cast<int>(std::floor(y - radius - 1));
    const int right = static_cast<int>(std::ceil(x + radius + 1));
    const int bottom = static_cast<int>(std::ceil(y + radius + 1));
    for (int py = top; py <= bottom; ++py) for (int px = left; px <= right; ++px) {
        const float dx = px + .5f - x, dy = py + .5f - y;
        const float coverage = std::clamp(radius + .5f - std::sqrt(dx * dx + dy * dy), 0.f, 1.f);
        over(px, py, config.meteorColor, alpha * coverage);
    }
}

void drawParticle(const Particle& p, int left, int top) {
    const float x = p.x - left, y = p.y - top;
    const float radius = config.meteorSize * .5f * p.scale;
    const float remaining = std::clamp(1.f - p.age / p.life, 0.f, 1.f);
    const float fade = remaining * remaining * config.meteorOpacity / 100.f;
    const float velocity = std::hypot(p.vx, p.vy);
    const float tail = 18.f * p.scale * std::clamp(config.meteorSpeed / 110.f, .6f, 1.7f);
    const int steps = static_cast<int>(std::ceil(tail));
    for (int step = steps; step > 0; --step) {
        const float t = static_cast<float>(step) / steps;
        dot(x - p.vx / velocity * tail * t, y - p.vy / velocity * tail * t,
            std::max(.4f, radius * .5f * (1.f - t)), fade * .48f * (1.f - t) * (1.f - t));
    }
    const int extent = static_cast<int>(std::ceil(radius * 2.8f));
    const int cx = static_cast<int>(std::floor(x)), cy = static_cast<int>(std::floor(y));
    for (int py = cy - extent; py <= cy + extent; ++py) for (int px = cx - extent; px <= cx + extent; ++px) {
        const float dx = px + .5f - x, dy = py + .5f - y;
        const float distance = std::sqrt(dx * dx + dy * dy);
        const float glow = .20f * std::exp(-(dx * dx + dy * dy) / (radius * radius * 1.5f));
        const float horizontal = std::clamp(1.f - std::abs(dx) / (radius * 1.2f), 0.f, 1.f) *
            std::clamp(1.f - std::abs(dy) / (.55f + radius * .10f), 0.f, 1.f);
        const float vertical = std::clamp(1.f - std::abs(dy) / (radius * 1.2f), 0.f, 1.f) *
            std::clamp(1.f - std::abs(dx) / (.55f + radius * .10f), 0.f, 1.f);
        const float cross = std::max(horizontal, vertical);
        over(px, py, config.meteorColor, fade * std::max(glow, cross * .9f));
        if (distance < std::max(.65f, radius * .28f)) over(px, py, 0xFFFFFF, fade * .95f);
    }
}

RECT particleBounds() {
    float left = particles[0].x, right = left, top = particles[0].y, bottom = top;
    for (std::size_t i = 0; i < count; ++i) {
        const auto& p = particles[i];
        const float padding = (18.f * std::clamp(config.meteorSpeed / 110.f, .6f, 1.7f) +
            config.meteorSize * 1.4f) * p.scale + 3.f;
        left = std::min(left, p.x - padding); right = std::max(right, p.x + padding);
        top = std::min(top, p.y - padding); bottom = std::max(bottom, p.y + padding);
    }
    return {static_cast<LONG>(std::floor(left)), static_cast<LONG>(std::floor(top)),
            static_cast<LONG>(std::ceil(right)), static_cast<LONG>(std::ceil(bottom))};
}

void dropOldest() {
    if (count) {
        for (std::size_t i = 1; i < count; ++i) particles[i - 1] = particles[i];
        --count;
    }
}

void emit(float x, float y, float scale) {
    if (count == MaxParticles) dropOldest();
    particles[count++] = {x, y, (35.f + 45.f * randomUnit()) / 110.f * scale,
        (70.f + 60.f * randomUnit()) / 110.f * scale, 0.f, .70f + .25f * randomUnit(), scale};
}
}

void resetMeteors() {
    count = 0; initialized = false; distanceCredit = 0.f;
    releaseCanvas();
}

void tickMeteors() {
    if (!enabled || !config.meteor) return;
    if (!config.meteorOpacity) { resetMeteors(); return; }
    POINT cursor{};
    if (!GetCursorPos(&cursor)) return;
    const ULONGLONG now = GetTickCount64();
    if (!initialized) {
        previousCursor = cursor; previousTick = now; initialized = true;
        randomState ^= static_cast<std::uint32_t>(now) | 1U;
        return;
    }
    const float elapsed = (now - previousTick) / 1000.f;
    previousTick = now;
    const float dt = std::min(elapsed, .05f);
    std::size_t alive = 0;
    for (std::size_t i = 0; i < count; ++i) {
        auto p = particles[i];
        p.age += elapsed;
        if (p.age >= p.life) continue;
        p.vy += 80.f / 110.f * p.scale * dt;
        p.x += p.vx * config.meteorSpeed * dt; p.y += p.vy * config.meteorSpeed * dt;
        particles[alive++] = p;
    }
    count = alive;
    const float dx = static_cast<float>(cursor.x - previousCursor.x);
    const float dy = static_cast<float>(cursor.y - previousCursor.y);
    const float distance = std::hypot(dx, dy);
    const float scale = std::max(1U, GetDpiForWindow(owner)) / 96.f;
    const float spacing = (42.f - .36f * config.meteorDensity) * scale;
    if (distance > 0.f) {
        distanceCredit += distance;
        const int available = static_cast<int>(distanceCredit / spacing);
        distanceCredit = std::fmod(distanceCredit, spacing);
        const int births = std::min(available, 8);
        for (int i = births - 1; i >= 0; --i) {
            const float position = std::clamp((distance - distanceCredit - i * spacing) / distance, 0.f, 1.f);
            emit(previousCursor.x + dx * position, previousCursor.y + dy * position, scale);
        }
        if (!births && !count) emit(static_cast<float>(cursor.x), static_cast<float>(cursor.y), scale);
    }
    previousCursor = cursor;
    if (!count) { releaseCanvas(); return; }
    RECT rect = particleBounds();
    while (count && (rect.right - rect.left > CanvasWidth || rect.bottom - rect.top > CanvasHeight)) {
        dropOldest();
        if (count) rect = particleBounds();
    }
    if (!count) { releaseCanvas(); return; }
    if (!ensureCanvas()) {
        resetMeteors(); config.meteor = false; refreshSettings();
        status(L"Cannot allocate meteor trail. Highlight is still available."); return;
    }
    drawWidth = rect.right - rect.left; drawHeight = rect.bottom - rect.top;
    for (int row = 0; row < drawHeight; ++row) std::fill_n(pixels + row * CanvasWidth, drawWidth, 0U);
    for (std::size_t i = 0; i < count; ++i) drawParticle(particles[i], rect.left, rect.top);
    POINT destination{rect.left, rect.top}, source{};
    SIZE size{drawWidth, drawHeight};
    BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    UpdateLayeredWindow(trailWindow, nullptr, &destination, &size, canvasDC, &source, 0, &blend, ULW_ALPHA);
    ShowWindow(trailWindow, SW_SHOWNOACTIVATE);
    SetWindowPos(trailWindow, HWND_TOPMOST, rect.left, rect.top, 0, 0,
        SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
}
}
