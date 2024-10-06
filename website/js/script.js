// 定义变量
let outputContent = '';
const outputElement = document.getElementById('output');
const statusBarElement = document.getElementById('statusBar');
const astContainerElement = document.getElementById('astContainer');
const byteCodeContainerElement = document.getElementById('byteCodeContainer');
let selectedCodeId = null;

// 初始化 CodeMirror 编辑器
const editor = CodeMirror(document.getElementById('editor'), {
    mode: 'javascript', // 根据需要更换语言模式
    theme: 'monokai',   // 选择主题
    lineNumbers: true,  // 显示行号
    fontSize: 14,
    value: '// 在这里输入代码'           // 初始内容为空，稍后根据参数设置
});

// 解析URL参数
function getQueryParam(param) {
    const urlParams = new URLSearchParams(window.location.search);
    return urlParams.get(param);
}

// 设置默认代码
function setDefaultCode() {
    const codeId = getQueryParam('codeId');
    if (codeId) {
        const codeItem = codeSelection.find(item => item.id === parseInt(codeId));
        if (codeItem) {
            editor.setValue(codeItem.codeContent);
            selectedCodeId = codeItem.id;
        }
    }
}

// 配置 Module 对象
var Module = {
    onRuntimeInitialized: function () {
        console.log('WebAssembly module loaded.');
    },
    print: function (text) {
        processOutput(text);
    },
    printErr: function (text) {
        updateStatusBar(`${text}`);
    }
};

// 监听控制台的警告输出
(function () {
    var originalConsoleWarn = console.warn;
    console.warn = function (message) {
        dispatchMessage(message);
        originalConsoleWarn.apply(console, arguments);
    };
})();

function dispatchMessage(message) {
    if (message.startsWith('[status]')) {
        updateStatusBar(message);
    } else if (message.startsWith('[ast]')) {
        updateAST(message);
    } else if (message.startsWith('[bytecode]')) {
        updateByteCode(message);
    }
}

// 更新状态栏信息
function updateStatusBar(message) {
    const timestamp = new Date().toLocaleTimeString('en-US', {hour12: false}) + '.' + new Date().getMilliseconds();
    let text = '';
    if (message.startsWith('[status]-')) {
        text = message.replace('[status]-', '');
    } else if (message.startsWith('[status][error]-')) {
        text = message.replace('[status][error]-', '');
        text = `<span class="error">${text}</span>`;
    } else if(message.startsWith('[status][info]-')){
        text = message.replace('[status][info]-', '');
        text = `<span class="info">${text}</span>`;
    }
    if (text === '') return;
    statusBarElement.innerHTML += `[${timestamp}] ${text}\n`;
}

function updateAST(message) {
    let astJson = message.replace('[ast]-', '');
    astContainerElement.innerHTML = `<span>----AST-----<br></span>` +
        '<pre>' + JSON.stringify(JSON.parse(astJson), null, 2) + '</pre>';
}

function updateByteCode(message) {
    let bytecode = message.replace('[bytecode]-', '');
    if (bytecode.endsWith('[nl]')) {
        bytecode = bytecode.slice(0, -4);
        byteCodeContainerElement.innerHTML += `<span>${bytecode}<br></span>`;
    } else {
        byteCodeContainerElement.innerHTML += `<span>${bytecode}</span>`;
    }
}

// 处理输出内容
function processOutput(text) {
    outputContent += text + '\n';
    outputElement.innerHTML = outputContent;
}

// 处理用户输入的代码并调用 WebAssembly 函数
function processInput() {
    const codeInput = editor.getValue();
    if (codeInput.trim() === "") {
        alert("请输入一些代码。");
        return;
    }

    // 清空之前的输出内容
    outputContent = '';
    outputElement.innerHTML = '';
    statusBarElement.innerHTML = '';
    astContainerElement.innerHTML = '';
    byteCodeContainerElement.innerHTML = '';

    // 调用 C 函数处理输入
    Module.ccall(
        'handle_file_content',  // C 函数名
        'void',                 // 返回值类型
        ['string'],             // 参数类型
        [codeInput]             // 参数值
    );

    // 去掉最后的 '\n'（如果有）
    if (outputContent.endsWith('\n')) {
        outputContent = outputContent.slice(0, -1);
        outputElement.innerHTML = outputContent;
    }
}

// 更新代码选择项，增加故事内容
const codeSelection = [
    {
        id: 1,
        name: "\u793a\u4f8b\u4ee3\u78011",
        intro: "\u6253\u5370Hello World",
        codeContent: 'code/code1.duo',
        storyContent: 'story/story1.html',
        enabled: true,
        thumbnail: "thumbnail/t1.png"
    },
    {
        id: 2,
        name: "\u793a\u4f8b\u4ee3\u78012",
        intro: "\u5faa\u73af\u6253\u5370\u6570\u5b57",
        codeContent: 'code/code2.duo',
        storyContent: 'story2.html',
        enabled: true,
        thumbnail: "thumbnail/t2.png"
    },
    {
        id: 3,
        name: "\u793a\u4f8b\u4ee3\u78012",
        intro: "\u5faa\u73af\u6253\u5370\u6570\u5b57",
        codeContent: 'code/code2.duo',
        storyContent: 'story2.html',
        enabled: true,
        thumbnail: "thumbnail/t3.png"
    },
    {
        id: 4,
        name: "\u793a\u4f8b\u4ee3\u78012",
        intro: "\u5faa\u73af\u6253\u5370\u6570\u5b57",
        codeContent: 'code/code2.duo',
        storyContent: 'story2.html',
        enabled: true,
        thumbnail: "thumbnail/t4.png"
    },
    {
        id: 5,
        name: "\u793a\u4f8b\u4ee3\u78012",
        intro: "\u5faa\u73af\u6253\u5370\u6570\u5b57",
        codeContent: 'code/code2.duo',
        storyContent: 'story2.html',
        enabled: true,
        thumbnail: "thumbnail/t5.webp"
    },
    {
        id: 6,
        name: "\u793a\u4f8b\u4ee3\u78012",
        intro: "\u5faa\u73af\u6253\u5370\u6570\u5b57",
        codeContent: 'code/code2.duo',
        storyContent: 'story2.html',
        enabled: true,
        thumbnail: "thumbnail/t6.webp"
    },
    {
        id: 7,
        name: "\u793a\u4f8b\u4ee3\u78012",
        intro: "\u5faa\u73af\u6253\u5370\u6570\u5b57",
        codeContent: 'code/code2.duo',
        storyContent: 'story2.html',
        enabled: true,
        thumbnail: "thumbnail/t7.webp"
    },
    {
        id: 8,
        name: "\u793a\u4f8b\u4ee3\u78012",
        intro: "\u5faa\u73af\u6253\u5370\u6570\u5b57",
        codeContent: 'code/code2.duo',
        storyContent: 'story2.html',
        enabled: true,
        thumbnail: "thumbnail/t8.webp"
    },
    {
        id: 9,
        name: "\u793a\u4f8b\u4ee3\u78012",
        intro: "\u5faa\u73af\u6253\u5370\u6570\u5b57",
        codeContent: 'code/code2.duo',
        storyContent: 'story2.html',
        enabled: true,
        thumbnail: "thumbnail/t9.webp"
    },
    {
        id: 10,
        name: "\u793a\u4f8b\u4ee3\u78012",
        intro: "\u5faa\u73af\u6253\u5370\u6570\u5b57",
        codeContent: 'code/code2.duo',
        storyContent: 'story2.html',
        enabled: true,
        thumbnail: "thumbnail/t10.webp"
    },
];

function selectCode() {
    const modal = document.createElement('div');
    modal.style.position = 'fixed';
    modal.style.top = '50%';
    modal.style.left = '50%';
    modal.style.transform = 'translate(-50%, -50%)';
    modal.style.backgroundColor = '#fff';
    modal.style.padding = '20px';
    modal.style.boxShadow = '0 2px 10px rgba(0,0,0,0.5)';
    modal.style.zIndex = '9999';
    modal.style.width = '600px';
    modal.style.height = '430px';
    modal.style.overflow = 'auto';

    codeSelection.forEach((codeItem) => {
        const card = document.createElement('div');
        card.className = 'card';
        card.innerHTML = `<img src="${codeItem.thumbnail}" alt="\u7f29\u7565\u56fe"><div><strong>${codeItem.name}</strong><br>${codeItem.intro}</div>`;
        if (selectedCodeId === codeItem.id) {
            card.classList.add('selected');
        }

        // 添加阅读按钮
        const readButton = document.createElement('button');
        readButton.textContent = '\u9605\u8bfb';
        readButton.className = 'readButton';
        readButton.onclick = function () {
            showStoryModal(codeItem.storyContent);
        };
        card.appendChild(readButton);

        card.onclick = function () {
            fetch(codeItem.codeContent)
                .then(response => response.text())
                .then(data => {
                    editor.setValue(data);
                    selectedCodeId = codeItem.id;
                    document.body.removeChild(modal);
                });
        };
        modal.appendChild(card);
    });

    document.body.appendChild(modal);
}

function showStoryModal(storyUrl) {
    fetch(storyUrl)
        .then(response => response.text())
        .then(storyContent => {
            const storyModal = document.createElement('div');
            storyModal.style.position = 'fixed';
            storyModal.style.top = '50%';
            storyModal.style.left = '50%';
            storyModal.style.transform = 'translate(-50%, -50%)';
            storyModal.style.backgroundColor = '#fff';
            storyModal.style.padding = '20px';
            storyModal.style.boxShadow = '0 2px 10px rgba(0,0,0,0.5)';
            storyModal.style.zIndex = '10000';
            storyModal.style.width = '80%';
            storyModal.style.height = '80%';
            storyModal.style.overflow = 'auto';
            storyModal.style.backgroundColor="#FDF5E6";

            // 添加关闭按钮 (仅右上角一个叉号图标)
            const closeButtonTop = document.createElement('button');
            closeButtonTop.className = 'closeButtonIcon';
            closeButtonTop.onclick = function () {
                document.body.removeChild(storyModal);
            };
            storyModal.appendChild(closeButtonTop);

            // 添加故事内容
            const content = document.createElement('div');
            content.innerHTML = storyContent;
            storyModal.appendChild(content);

            document.body.appendChild(storyModal);
        });
}

// 设置默认代码
setDefaultCode();