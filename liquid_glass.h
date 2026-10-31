// liquid_glass.h - Real optical liquid glass & backdrop refraction for Dynamic Island
//
// Captures the live desktop pixels behind the island, applies a fast separable
// optical blur, lens edge refraction (light curvature), and renders through Direct2D
// with subpixel antialiased rounded pill corners.
#pragma once
#include <windows.h>
#include <d2d1.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <memory>

namespace liquid_glass {

struct GlassEngine {
    HDC memDc_ = nullptr;
    HBITMAP hBitmap_ = nullptr;
    HBITMAP oldBitmap_ = nullptr;
    uint32_t* rawBits_ = nullptr;
    int bufWidth_ = 0;
    int bufHeight_ = 0;

    std::vector<uint32_t> blurTemp_;
    std::vector<uint32_t> refractedBits_;

    Microsoft::WRL::ComPtr<ID2D1Bitmap> d2dBitmap_;
    POINT lastCaptureOrigin_ = {-9999, -9999};
    int lastCaptureW_ = 0;
    int lastCaptureH_ = 0;
    double lastCaptureTime_ = 0.0;

    void Release() {
        if (memDc_) {
            if (oldBitmap_) SelectObject(memDc_, oldBitmap_);
            DeleteDC(memDc_);
            memDc_ = nullptr;
        }
        if (hBitmap_) {
            DeleteObject(hBitmap_);
            hBitmap_ = nullptr;
        }
        rawBits_ = nullptr;
        bufWidth_ = 0;
        bufHeight_ = 0;
        d2dBitmap_.Reset();
    }

    ~GlassEngine() {
        Release();
    }

    bool EnsureBuffers(int w, int h) {
        if (memDc_ && hBitmap_ && bufWidth_ == w && bufHeight_ == h) {
            return true;
        }
        Release();

        HDC screenDc = GetDC(nullptr);
        if (!screenDc) return false;

        memDc_ = CreateCompatibleDC(screenDc);
        if (!memDc_) {
            ReleaseDC(nullptr, screenDc);
            return false;
        }

        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = w;
        bmi.bmiHeader.biHeight = -h;  // top-down DIB
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        hBitmap_ = CreateDIBSection(memDc_, &bmi, DIB_RGB_COLORS, reinterpret_cast<void**>(&rawBits_), nullptr, 0);
        ReleaseDC(nullptr, screenDc);

        if (!hBitmap_ || !rawBits_) {
            Release();
            return false;
        }

        oldBitmap_ = reinterpret_cast<HBITMAP>(SelectObject(memDc_, hBitmap_));
        bufWidth_ = w;
        bufHeight_ = h;
        blurTemp_.resize(w * h);
        refractedBits_.resize(w * h);
        return true;
    }

    // Fast separable sliding-box blur (approximates Gaussian with 2 passes in ~0.2ms)
    void ApplyBlur(int w, int h, int radius) {
        if (radius <= 0 || !rawBits_) return;

        uint32_t* src = rawBits_;
        uint32_t* tmp = blurTemp_.data();
        const int div = 2 * radius + 1;

        // Horizontal pass
        for (int y = 0; y < h; ++y) {
            const int rowStart = y * w;
            int rSum = 0, gSum = 0, bSum = 0;

            // Preload window
            for (int i = -radius; i <= radius; ++i) {
                const int clampedX = std::clamp(i, 0, w - 1);
                const uint32_t pix = src[rowStart + clampedX];
                bSum += (pix & 0xFF);
                gSum += ((pix >> 8) & 0xFF);
                rSum += ((pix >> 16) & 0xFF);
            }

            for (int x = 0; x < w; ++x) {
                tmp[rowStart + x] = 0xFF000000 |
                    ((rSum / div) << 16) |
                    ((gSum / div) << 8) |
                    (bSum / div);

                const int xOut = std::clamp(x - radius, 0, w - 1);
                const int xIn = std::clamp(x + radius + 1, 0, w - 1);
                const uint32_t pOut = src[rowStart + xOut];
                const uint32_t pIn = src[rowStart + xIn];

                bSum += (pIn & 0xFF) - (pOut & 0xFF);
                gSum += ((pIn >> 8) & 0xFF) - ((pOut >> 8) & 0xFF);
                rSum += ((pIn >> 16) & 0xFF) - ((pOut >> 16) & 0xFF);
            }
        }

        // Vertical pass
        for (int x = 0; x < w; ++x) {
            int rSum = 0, gSum = 0, bSum = 0;

            for (int i = -radius; i <= radius; ++i) {
                const int clampedY = std::clamp(i, 0, h - 1);
                const uint32_t pix = tmp[clampedY * w + x];
                bSum += (pix & 0xFF);
                gSum += ((pix >> 8) & 0xFF);
                rSum += ((pix >> 16) & 0xFF);
            }

            for (int y = 0; y < h; ++y) {
                src[y * w + x] = 0xFF000000 |
                    ((rSum / div) << 16) |
                    ((gSum / div) << 8) |
                    (bSum / div);

                const int yOut = std::clamp(y - radius, 0, h - 1);
                const int yIn = std::clamp(y + radius + 1, 0, h - 1);
                const uint32_t pOut = tmp[yOut * w + x];
                const uint32_t pIn = tmp[yIn * w + x];

                bSum += (pIn & 0xFF) - (pOut & 0xFF);
                gSum += ((pIn >> 8) & 0xFF) - ((pOut >> 8) & 0xFF);
                rSum += ((pIn >> 16) & 0xFF) - ((pOut >> 16) & 0xFF);
            }
        }
    }

    // Optical lens refraction: light gently bends around the pill glass capsule geometry
    void ApplyRefraction(int w, int h, float cornerRadius) {
        if (!rawBits_ || w <= 8 || h <= 8) return;

        const float r = std::min(cornerRadius, h * 0.5f);
        const float cy = h * 0.5f;
        const float maxDisplacement = 4.0f; // clean, subtle physical optical distortion in pixels
        uint32_t* src = rawBits_;
        uint32_t* dst = refractedBits_.data();

        for (int y = 0; y < h; ++y) {
            const float dy = y - cy;
            const int row = y * w;

            for (int x = 0; x < w; ++x) {
                float distFromCenter = 0.0f;
                float normalX = 0.0f;
                float normalY = (h > 0) ? (dy / (h * 0.5f)) : 0.0f;

                if (x < r) {
                    // Left rounded endcap
                    const float dx = x - r;
                    distFromCenter = std::sqrt(dx * dx + dy * dy);
                    if (distFromCenter > 0.001f && r > 0.0f) {
                        normalX = dx / r;
                        normalY = dy / r;
                    }
                } else if (x > w - r) {
                    // Right rounded endcap
                    const float dx = x - (w - r);
                    distFromCenter = std::sqrt(dx * dx + dy * dy);
                    if (distFromCenter > 0.001f && r > 0.0f) {
                        normalX = dx / r;
                        normalY = dy / r;
                    }
                } else {
                    // Straight center section: displacement strictly vertical towards top/bottom
                    distFromCenter = std::abs(dy);
                    normalX = 0.0f;
                }

                const float normDist = (h > 0) ? (distFromCenter / (h * 0.5f)) : 0.0f;
                if (normDist > 0.50f) {
                    const float factor = std::min(1.0f, (normDist - 0.50f) / 0.50f);
                    const float shift = factor * maxDisplacement;
                    const int sx = std::clamp(static_cast<int>(x + normalX * shift), 0, w - 1);
                    const int sy = std::clamp(static_cast<int>(y + normalY * shift), 0, h - 1);
                    dst[row + x] = src[sy * w + sx];
                } else {
                    dst[row + x] = src[row + x];
                }
            }
        }
    }

    // Captures the screen behind the pill, blurs it, applies refraction, and creates a Direct2D bitmap
    ID2D1Bitmap* UpdateAndGetBitmap(ID2D1RenderTarget* target, POINT screenOrigin,
                                    float pillLeft, float pillTop, float pillW, float pillH,
                                    float cornerRadius, double now) {
        const int w = static_cast<int>(std::ceil(pillW));
        const int h = static_cast<int>(std::ceil(pillH));
        if (w < 8 || h < 8 || !target) return nullptr;

        const int screenX = screenOrigin.x + static_cast<int>(std::floor(pillLeft));
        const int screenY = screenOrigin.y + static_cast<int>(std::floor(pillTop));

        const bool sizeChanged = (w != lastCaptureW_ || h != lastCaptureH_);
        const bool posChanged = (screenX != lastCaptureOrigin_.x || screenY != lastCaptureOrigin_.y);
        // Throttle background recapture to ~15Hz to keep CPU impact negligible
        const bool timeToRecapture = (now - lastCaptureTime_) > 0.066;

        if (d2dBitmap_ && !sizeChanged && !posChanged && !timeToRecapture) {
            return d2dBitmap_.Get();
        }

        if (!EnsureBuffers(w, h)) return nullptr;

        HDC screenDc = GetDC(nullptr);
        if (!screenDc) return nullptr;

        // BitBlt captures windows and wallpaper behind this layered window (WS_EX_LAYERED is excluded by default)
        BitBlt(memDc_, 0, 0, w, h, screenDc, screenX, screenY, SRCCOPY);
        ReleaseDC(nullptr, screenDc);

        // Apply 2-pass high quality optical blur (radius 10px)
        ApplyBlur(w, h, 10);

        // Apply liquid glass lens refraction displacement
        ApplyRefraction(w, h, cornerRadius);

        // Upload to Direct2D hardware bitmap
        D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties(
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE));

        if (!d2dBitmap_ || sizeChanged) {
            d2dBitmap_.Reset();
            HRESULT hr = target->CreateBitmap(D2D1::SizeU(w, h), refractedBits_.data(), w * sizeof(uint32_t),
                                             props, d2dBitmap_.GetAddressOf());
            if (FAILED(hr)) return nullptr;
        } else {
            D2D1_RECT_U destRect = D2D1::RectU(0, 0, w, h);
            d2dBitmap_->CopyFromMemory(&destRect, refractedBits_.data(), w * sizeof(uint32_t));
        }

        lastCaptureOrigin_ = {screenX, screenY};
        lastCaptureW_ = w;
        lastCaptureH_ = h;
        lastCaptureTime_ = now;

        return d2dBitmap_.Get();
    }
};

inline GlassEngine g_glassEngine;

}  // namespace liquid_glass
