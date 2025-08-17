基于 esp_http_server 的一些帮助函数

proxy.c 可能没完成好，本意是想通过 ESP32 设备转发一些原本在网页上因 CROS 错误无法获取的内容

util.js 里除了一些简单的js 帮助函数外，还有 j2h 的部分，用于将js 对象转化为 html 标签，见 https://github.com/hyansuper/j2h