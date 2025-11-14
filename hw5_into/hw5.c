#include <stdio.h>
#include <dos.h>
#include <stdlib.h>
#include <conio.h>

/* 
 * 溢出中断处理作业 - 完整实现
 * 重写4号中断（INTO中断）服务程序
 */

// 全局变量记录溢出状态和信息
int overflow_detected = 0;
char error_message[128];
unsigned long overflow_address = 0;

// 保存原始中断向量
void interrupt (*original_overflow_isr)(void);

/* 
 * 新的溢出中断服务程序
 * 使用interrupt关键字让编译器自动处理寄存器保存和恢复
 */
void interrupt new_overflow_isr(void)
{
    // 设置溢出标志
    overflow_detected = 1;
    
    // 记录溢出发生的信息
    sprintf(error_message, "算术溢出错误：有符号数运算结果超出表示范围");
    
    // 在实际系统中，这里可以获取发生溢出的指令地址
    // 这里简化处理，只记录基本错误信息
    
    printf("\n*** 溢出中断被触发！ ***\n");
    printf("错误信息: %s\n", error_message);
    printf("请检查相关变量的值是否过大或过小\n");
    
    // 注意：中断处理程序不应该使用复杂的库函数调用
    // 这里为了演示使用printf，实际应用中应该更谨慎
}

/* 
 * 安全的有符号数加法函数
 * 使用INTO指令检测溢出
 */
int safe_signed_add(int a, int b)
{
    int result;
    overflow_detected = 0;  // 重置溢出标志
    
    // 使用内联汇编进行加法并检查溢出
    asm {
        mov eax, a      ; 将第一个操作数加载到EAX
        add eax, b      ; 执行加法，硬件会自动设置OF标志
        into            ; 如果OF=1则触发4号中断
        mov result, eax ; 保存结果
    }
    
    return result;
}

/* 
 * 安全的有符号数减法函数
 */
int safe_signed_sub(int a, int b)
{
    int result;
    overflow_detected = 0;
    
    asm {
        mov eax, a
        sub eax, b      ; 执行减法
        into            ; 检查溢出
        mov result, eax
    }
    
    return result;
}

/* 
 * 安全的有符号数乘法函数
 */
int safe_signed_mul(int a, int b)
{
    int result;
    overflow_detected = 0;
    
    asm {
        mov eax, a
        imul eax, b     ; 有符号数乘法
        into            ; 检查溢出
        mov result, eax
    }
    
    return result;
}

/* 
 * 安装新的中断服务程序
 */
void install_overflow_handler(void)
{
    printf("安装新的溢出中断服务程序...\n");
    
    // 保存原始中断向量
    original_overflow_isr = _dos_getvect(4);
    
    // 设置新的中断向量
    _dos_setvect(4, new_overflow_isr);
    
    printf("溢出中断处理程序安装完成！\n");
}

/* 
 * 恢复原始中断服务程序
 */
void restore_original_handler(void)
{
    printf("恢复原始中断服务程序...\n");
    _dos_setvect(4, original_overflow_isr);
    printf("原始中断服务程序已恢复！\n");
}

/* 
 * 测试函数 - 各种边界情况测试
 */
void run_tests(void)
{
    printf("\n========== 开始溢出测试 ==========\n");
    
    // 测试用例数组
    struct {
        int a, b;
        char operation; // '+', '-', '*'
        char *description;
    } test_cases[] = {
        {2147483647, 1, '+', "INT_MAX + 1 (正溢出)"},
        {-2147483648, -1, '+', "INT_MIN + (-1) (负溢出)"},
        {-2147483648, 1, '-', "INT_MIN - 1 (下溢)"},
        {2147483647, -1, '-', "INT_MAX - (-1) (上溢)"},
        {100000, 100000, '*', "大数乘法溢出"},
        {100, 200, '+', "正常加法测试"},
        {-100, -200, '+', "正常负数加法"},
        {500, 300, '-', "正常减法测试"}
    };
    
    int test_count = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (int i = 0; i < test_count; i++) {
        printf("\n测试 %d: %s\n", i + 1, test_cases[i].description);
        printf("操作: %d %c %d\n", 
               test_cases[i].a, 
               test_cases[i].operation, 
               test_cases[i].b);
        
        overflow_detected = 0;  // 重置标志
        
        int result;
        switch (test_cases[i].operation) {
            case '+':
                result = safe_signed_add(test_cases[i].a, test_cases[i].b);
                break;
            case '-':
                result = safe_signed_sub(test_cases[i].a, test_cases[i].b);
                break;
            case '*':
                result = safe_signed_mul(test_cases[i].a, test_cases[i].b);
                break;
        }
        
        if (overflow_detected) {
            printf("结果: 检测到溢出！实际结果: %d (不可信)\n", result);
            printf("建议: 使用long long类型或检查输入范围\n");
        } else {
            printf("结果: %d (正常)\n", result);
        }
        
        // 暂停一下，方便观察输出
        printf("按任意键继续下一个测试...");
        getch();
        printf("\n");
    }
}

/* 
 * 交互式测试模式
 */
void interactive_test(void)
{
    printf("\n========== 交互式测试模式 ==========\n");
    printf("输入两个整数进行有符号数加法测试\n");
    printf("输入 q 退出交互模式\n\n");
    
    while (1) {
        int a, b;
        char input[100];
        
        printf("请输入第一个整数: ");
        if (scanf("%s", input) != 1) break;
        
        // 检查是否退出
        if (input[0] == 'q' || input[0] == 'Q') break;
        
        a = atoi(input);
        
        printf("请输入第二个整数: ");
        if (scanf("%s", input) != 1) break;
        
        // 检查是否退出
        if (input[0] == 'q' || input[0] == 'Q') break;
        
        b = atoi(input);
        
        printf("\n计算: %d + %d\n", a, b);
        
        overflow_detected = 0;
        int result = safe_signed_add(a, b);
        
        if (overflow_detected) {
            printf("*** 溢出警告！ ***\n");
            printf("运算结果: %d (由于溢出，此结果不正确)\n", result);
            printf("请使用更大范围的数据类型\n");
        } else {
            printf("运算结果: %d (正常)\n", result);
        }
        
        printf("\n");
    }
}

/* 
 * 演示C语言默认的溢出行为
 */
void demonstrate_c_overflow_behavior(void)
{
    printf("\n========== C语言默认溢出行为演示 ==========\n");
    
    int max_int = 2147483647;
    int min_int = -2147483648;
    
    printf("C语言对有符号数溢出的处理是未定义行为(UB)\n");
    printf("INT_MAX = %d\n", max_int);
    printf("INT_MIN = %d\n", min_int);
    
    // C语言默认不会检测溢出
    printf("\nC语言默认行为（危险）:\n");
    printf("INT_MAX + 1 = %d (静默溢出)\n", max_int + 1);
    printf("INT_MIN - 1 = %d (静默溢出)\n", min_int - 1);
    
    printf("\n使用我们的安全函数:\n");
    safe_signed_add(max_int, 1);
    if (overflow_detected) {
        printf("检测到溢出并进行了处理！\n");
    }
}

/* 
 * 主函数
 */
int main(void)
{
    printf("===========================================\n");
    printf("   溢出中断服务程序重写 - 作业实现\n");
    printf("===========================================\n");
    
    // 安装中断处理程序
    install_overflow_handler();
    
    int choice;
    do {
        printf("\n请选择测试模式:\n");
        printf("1. 自动运行所有测试用例\n");
        printf("2. 交互式测试\n");
        printf("3. 演示C语言默认溢出行为\n");
        printf("4. 退出程序\n");
        printf("请输入选择 (1-4): ");
        
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                run_tests();
                break;
            case 2:
                interactive_test();
                break;
            case 3:
                demonstrate_c_overflow_behavior();
                break;
            case 4:
                printf("退出程序...\n");
                break;
            default:
                printf("无效选择，请重新输入！\n");
        }
    } while (choice != 4);
    
    // 恢复原始中断处理程序
    restore_original_handler();
    
    printf("\n程序结束，感谢使用！\n");
    return 0;
}
