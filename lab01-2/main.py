import cv2


def getD(input):
    h, w = len(input), len(input[0])
    status = 1 
    for i in range(h):
        for j in range(w):
            for k in range(0, 2):
                if input[i][j][k] != input[i][j][k + 1]:
                    status = 3
                    return status
    return status



def conv(input, m, n, kernel, bias, padding):
    h, w = len(input), len(input[0])
    d = getD(input) 

    background = []
    result = []
    
    if padding == -1:
        background = input
    
    else:
        padding_color = [255 for _ in range(d)] if padding == 1 else [0 for _ in range(d)]
        background = [[padding_color for _ in range(w + 2 * (m - 1))] for _ in range(h + 2 * ( n - 1 ))]
        cur_h = 0
        cur_w = 0

        # [)
        for i in range(n-1, n + h - 1):
            cur_w = 0
            for j in range(m - 1, m + w - 1):
                background[i][j] = input[cur_h][cur_w]
                cur_w += 1
            cur_h += 1


    for i in range(0, len(background) - n + 1):
        for j in range(0,  len(background[0]) - m + 1):
            new_row = []
            row_val = 0
            kernel_y = 0
            kernel_x = 0
            for col in range(i, i + n):
                sub_row = 0
                for row in range(j, j + m):
                    if d == 1:
                        sub_row += background[col][row][0] * kernel[kernel_y][kernel_x]
                    else:
                        for k in range(d):
                            sub_row += background[col][row][k] * kernel[kernel_y][kernel_x][k]
                        sub_row /= 3
                    row_val += sub_row
                    sub_row = 0
                    kernel_x += 1
                kernel_y += 1
                kernel_x = 0
                insert_val = row_val + bias
            new_row.append([insert_val for _ in range(3)])
        result.append(new_row)


    return result

            

def main():
    img = cv2.imread("./images/duck.jpeg")
    if img is None:
        print("Can't find image.")
        return

    m = n = 3
    d = 3
    kernel = [[[0 for _ in range(d)] for _ in range(m)] for _ in range(n)]
    padding = 1
    bias = 0

    result = conv(img, m, n, kernel, bias, padding)

    cv2.imshow("Origional Image", img)
    print(result)
    # cv2.imshow("Processing Image", result)
    cv2.waitKey(0)

if __name__ == "__main__":
    main()
