/**
 * @file hal_comm_s.c
 * @author 文佳源，绕洪江
 * @brief hal层通用源文件，通用即与硬件细节无关
 * @version 1.0
 * @date 2025-05-10
 * 
 * @copyright Copyright (c) 2025
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>文佳源，绕洪江 <td>2025-05-10 <td>创建文件
 */

.global HAL_GET_CURRENT_CPU

HAL_GET_CURRENT_CPU:
    rd  %asr17, %l0
    srl %l0, 28, %o0
    retl
    nop 
