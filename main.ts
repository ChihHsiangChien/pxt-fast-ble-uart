/**
 * 高頻率藍牙傳輸擴充套件
 */
//% color="#FF4500" icon="\uf293" block="Fast BLE"
namespace fastble {
    /**
     * 啟動背景 1000Hz 採樣任務與高速藍牙發送
     */
    //% block="啟動高速傳輸服務"
    export function startCapture(): void {
        // 在硬體上這會呼叫 C++ 的實作，在模擬器上則執行這裡的代碼
        startCaptureNative();
    }

    //% shim=fastble::startCaptureNative
    function startCaptureNative(): void {
        // 模擬器 fallback
        console.log("Fast BLE: startCapture is not supported in simulator");
    }
}
