# micro:bit 高頻藍牙 UART 傳輸擴充套件 (pxt-fast-ble-uart)

這是一個專為 micro:bit V1 與 V2 設計的高效能藍牙傳輸擴充套件。它能夠以 **1000Hz (1ms)** 的採樣率讀取感測器數據，並透過最佳化的自定義藍牙 UART 服務，穩定地將大量數據發送至接收端（如手機 App 或電腦）。

## 主要功能

*   **1000Hz 高頻採樣**：利用 C++ Fiber 背景任務實現每毫秒一次的感測器讀取，遠高於一般的 TypeScript 輪詢速度。
*   **自定義 Fast UART 服務**：
    *   內建更大的快取區 (2048 bytes)，防止高速傳輸時數據遺失。
    *   最佳化的 MTU 與 Notify 機制。
*   **封裝傳輸協定**：
    *   每 20 筆採樣數據（每筆 6 bytes，共 120 bytes）封裝成一個 128 bytes 的封包。
    *   封包末尾包含 4 bytes 的全域序號 (Sample Index)，方便接收端檢查是否丟包。
*   **自動感測器偵測**：
    *   支援 I2C 位址 `0x32` (LIS3DH 系列) 與 `0x24/0x26` (QMA 系列) 加速度計。
    *   自動初始化感測器量程 (±2g, ±4g, ±8g, ±16g)。
*   **互動反饋**：
    *   **唯一識別名稱**：啟動高速傳輸時，LED 矩陣會自動捲動顯示 micro:bit 的 5 字元唯一名稱 (Friendly Name, 例如 `zoged`)，方便在藍牙搜尋時辨識對應的機器。
    *   按下按鈕 A：循環切換加速度計量程。
    *   按下按鈕 B：開啟/關閉 LED 矩陣顯示（顯示當前量程與採樣狀態）。

## 使用方法

### 導入擴充功能

1.  開啟 [MakeCode for micro:bit](https://makecode.microbit.org/)。
2.  點擊 **擴充功能 (Extensions)**。
3.  在搜索框中貼上：`https://github.com/chihhsiangchien/pxt-fast-ble-uart`。
4.  點擊匯入。

### 積木使用

在 `當啟動時` 放入 **[啟動高速傳輸服務]** 積木即可：

```blocks
fastble.startCapture()
```

## 技術規格

*   **傳輸速率**：1000Hz 採樣，每 20ms 發送一個 128 bytes 封包（約 6.4 KB/s 原始數據）。
*   **封包結構 (128 bytes)**：
    *   `[0-119]`：20 組 X, Y, Z 加速度原始數據 (16-bit signed, Little Endian)。
    *   `[120-123]`：保留位。
    *   `[124-127]`：32-bit Big Endian Sample Index。

## 平台支援

*   **micro:bit V1** (DAL)
*   **micro:bit V2** (CODAL) - 推薦使用以獲得最佳效能。

---

> 在 [https://chihhsiangchien.github.io/pxt-fast-ble-uart/](https://chihhsiangchien.github.io/pxt-fast-ble-uart/) 打開此頁面

#### 中繼資料 (用於搜索、渲染)

* for PXT/microbit
<script src="https://makecode.com/gh-pages-embed.js"></script><script>makeCodeRender("{{ site.makecode.home_url }}", "{{ site.github.owner_name }}/{{ site.github.repository_name }}");</script>
