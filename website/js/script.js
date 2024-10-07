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
            fetch(codeItem.codeContent)
                .then(response => response.text())
                .then(data => {
                    editor.setValue(data);
                    selectedCodeId = codeItem.id;
                });
        }
    }
}

// 配置 Module 对象
var Module = {
    onRuntimeInitialized: function () {
        console.log('WebAssembly module loaded.');
    }
};

// 监听控制台的警告输出
(function () {
    var originalConsoleWarn = console.warn;
    console.warn = function (message) {
        dispatchMessage(message);
        // originalConsoleWarn.apply(console, arguments);
    };
})();


// 监听控制台的警告输出
(function () {
    var originalConsoleInfo = console.info;
    console.info = function (message) {
        updateConsole(message);
        // originalConsoleInfo.apply(console, arguments);
    };
})();

function updateConsole(message){
    outputElement.value += message;
}

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
    const date = new Date();
    const timestamp = date.getFullYear() + '/' +
        String(date.getMonth() + 1).padStart(2, '0') + '/' +
        String(date.getDate()).padStart(2, '0') + ' ' +
        date.toLocaleTimeString('en-US', {hour12: false}) + '.' +
        date.getMilliseconds();

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

// 处理用户输入的代码并调用 WebAssembly 函数
function processInput() {
    const codeInput = editor.getValue();
    if (codeInput.trim() === "") {
        alert("请输入一些代码。");
        return;
    }

    // 清空之前的输出内容
    outputContent = '';
    outputElement.value = '';
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
    // if (outputContent.endsWith('\n')) {
    //     outputContent = outputContent.slice(0, -1);
    //     outputElement.innerHTML = outputContent;
    // }
}

// 更新代码选择项，增加故事内容
const codeSelection = [
    {
        id: 1,
        name: "0x00. 咦，这是什么？",
        intro: "我在哪里，这是什么？",
        codeContent: 'code/code1.duo',
        storyContent: 'story/story1.html',
        enabled: true,
        thumbnail: "thumbnail/t1.png"
    },
    {
        id: 2,
        name: "0x01. 你好，世界！",
        intro: "欢迎朵朵来到爸爸妈妈的身边，让我们一起进入神奇的魔法世界！",
        codeContent: 'code/code2.duo',
        storyContent: 'story/story2.html',
        enabled: true,
        thumbnail: "thumbnail/t2.png"
    },
    {
        id: 3,
        name: "0x02. 符号森林的神秘咒语",
        intro: "在符号森林中，小魔法师朵朵学习如何通过表达式和语句施展魔法，并发现了作用域的秘密——魔法只能在它所属的范围内生效",
        codeContent: 'code/code3.duo',
        storyContent: 'story/story3.html',
        enabled: true,
        thumbnail: "thumbnail/t3.png"
    },
    {
        id: 4,
        name: "0x03. 元素的守护者",
        intro: "朵朵在魔法学院中学习如何使用基础的元素力量，像数字、文字和真理之光，并掌握了用魔法钥匙（变量）来封存这些力量的技巧",
        codeContent: 'code/code4.duo',
        storyContent: 'story/story4.html',
        enabled: true,
        thumbnail: "thumbnail/t4.png"
    },
    {
        id: 5,
        name: "0x04. 选择的岔路与旋转的花园",
        intro: "朵朵在她的冒险中来到了“选择之路”，学习了如何通过愿望树作出不同的决定。她还发现了一个神奇的旋转花园，在那里她不断地循环，直到找到正确的出口",
        codeContent: 'code/code5.duo',
        storyContent: 'story/story5.html',
        enabled: true,
        thumbnail: "thumbnail/t5.webp"
    },
    {
        id: 6,
        name: "0x05. 魔法师的咒语与神秘的魔咒瓶",
        intro: "朵朵在她的魔法旅途中，学会了如何使用魔法咒语（函数）解决问题，探索了返回值、值传递、递归等魔法技巧。她还发现了一种神秘的力量——魔咒瓶（闭包），能够保存并延续魔法的效果",
        codeContent: 'code/code6.duo',
        storyContent: 'story/story6.html',
        enabled: true,
        thumbnail: "thumbnail/t6.webp"
    },
    {
        id: 7,
        name: "0x06. 城堡中的秘密法术",
        intro: "朵朵学习了如何通过类和实例创建强大的骑士，并理解了如何使用不同的方法和继承技巧扩展骑士的能力。她还发现了静态方法和多态的奥秘",
        codeContent: 'code/code7.duo',
        storyContent: 'story/story7.html',
        enabled: true,
        thumbnail: "thumbnail/t7.webp"
    },
    {
        id: 8,
        name: "0x07. 幻影施法者的秘密与纯净魔法的力量",
        intro: "朵朵在她的冒险中，遇到了一群神秘的施法者，他们的魔法不依赖于物理世界，而是通过纯粹的逻辑和函数实现。" ,
        codeContent: 'code/code8.duo',
        storyContent: 'story/story8.html',
        enabled: true,
        thumbnail: "thumbnail/t8.webp"
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
        card.style.display = 'flex';  // 使用 flexbox 布局使卡片内元素水平排列
        card.style.alignItems = 'center';
        card.style.padding = '15px';
        card.style.marginBottom = '15px';
        card.style.border = '1px solid #ccc';
        card.style.borderRadius = '8px';
        if (selectedCodeId === codeItem.id) {
            card.classList.add('selected');
        }

        // 图像部分
        // 图像部分
        const thumbnail = document.createElement('img');
        thumbnail.src = codeItem.thumbnail;
        thumbnail.alt = '\u7f29\u7565\u56fe';
        thumbnail.style.width = '100px'; // 设置固定宽度
        thumbnail.style.height = '100px'; // 设置固定高度
        thumbnail.style.marginRight = '15px';
        thumbnail.style.flexShrink = '0'; // 防止图像被压缩


        // 文字内容部分
        const textContainer = document.createElement('div');
        textContainer.style.flex = '4'; // 让文字部分占据大部分可用空间
        textContainer.style.paddingRight = '15px';
        textContainer.innerHTML = `<strong>${codeItem.name}</strong><br>${codeItem.intro}`;

        // 添加阅读按钮容器
        const buttonContainer = document.createElement('div');
        buttonContainer.style.display = 'flex'; // 改为 inline-flex，使容器宽度紧贴内容
        buttonContainer.style.justifyContent = 'center';
        buttonContainer.style.alignItems = 'center';
        buttonContainer.style.flexShrink = '0'; // 防止按钮容器被压缩
        buttonContainer.style.flex = '1'; // 让按钮容器占据较小部分空间

        const readButton = document.createElement('button');
        readButton.textContent = '\u9605\u8bfb';
        readButton.className = 'readButton';
        readButton.style.padding = '10px 20px';
        readButton.onclick = function () {
            showStoryModal(codeItem.storyContent);
        };
        buttonContainer.appendChild(readButton);



        // 将各个部分添加到卡片中
        card.appendChild(thumbnail);
        card.appendChild(textContainer);
        card.appendChild(buttonContainer);

        // 卡片点击事件
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