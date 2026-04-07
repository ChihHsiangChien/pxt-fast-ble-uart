declare namespace fastble {
    /**
     * 啟動背景 1000Hz 採樣任務與高速藍牙發送
     */
    //% block="啟動高速傳輸服務"
    //% shim=fastble::startCapture
    function startCapture(): void;
}