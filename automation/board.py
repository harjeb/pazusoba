# %%
import pyautogui as gui
import cv2 as cv
import numpy as np
import subprocess, os
import time
from airtest.core.api import *
import logging
from pic import Phone
from airtest.aircv import *



logger = logging.getLogger("airtest")
logger.setLevel(logging.ERROR)

os.system('adb connect 192.168.2.101:5555')
connect_device("Android:///")

dev = device()
#dev.swipe_along([[959, 418],[1157, 564],[1044, 824],[751, 638],[945, 415]],duration=0.1)
# %%
# start and end loc
#x = ''
#adb =ADB(x)

# 30 格 盘子
#board_loc =[162, 580, 592, 936]
board_loc = [7, 1390, 1070, 2274]
# what's the size of the board
# TODO: detect this automatically
board_config = [6, 5]
board_size = (830, 690)
orb_size = (140, 140)
border_len = 1
board_column = board_row = 0
screen_scale = 1
# get board position here
pos = gui.position()
# this makes update board_loc a bit easier
print("{}, {}".format(pos.x, pos.y))

# This has Red, Blue, Green, Light, Dark and Heal
colour_range = {
    "R": (np.array([0, 100, 100]), np.array([10, 255, 255])),
    "B": (np.array([100, 100, 100]), np.array([125, 255, 255])),
    "G": (np.array([40, 100, 100]), np.array([70, 255, 255])),
    "L": (np.array([20, 100, 100]), np.array([35, 255, 255])),
    "D": (np.array([125, 100, 100]), np.array([150, 255, 255])),
    "H": (np.array([150, 100, 100]), np.array([165, 255, 255])),
}

# read in some template orbs
thispath = os.path.abspath(os.path.dirname(__file__))
jammer_template = cv.imread(thispath+"\\template\\jammer.png", 0)
poison_template = cv.imread(thispath+"\\template\\poison.png", 0)
bomb_template = cv.imread(thispath+"\\template\\bomb.png", 0)
heal_template = cv.imread(thispath+"\\template\\heal.png", 0)
orb_templates = {
    "J": (jammer_template, 0.6),
    "P": (poison_template, 0.6),
    "H": (heal_template, 0.6),
    "E": (bomb_template, 0.7) # E for explosive
}
match_offset = 25

# %%
# based on count, get a list of every orb, this is also an ordered list
def getEachOrb(image):
    # weight, height
    w, h = board_size
    orbs = []

    global board_column, board_row
    if orb_count == 20:
        # 5x4
        board_column = 5
        board_row = 4
    elif orb_count == 30:
        # 6x5
        board_column = 6
        board_row = 5
    elif orb_count == 42:
        # 7x6
        board_column = 7
        board_row = 6
    else:
        print("Unknown orb count.")

    # consider added padding here
    initial = border_len * 2
    orb_w = int((w + initial) / board_column)
    orb_h = int((h + initial) / board_row)
    x1 = y1 = initial
    x2 = y2 = initial
    # start getting every orb
    for i in range(board_row):
        # update x2 and y2 first
        y2 += orb_h
        for j in range(board_column):
            x2 += orb_w
            # NOTE: you need to save a copy here because it changes image
            orbs.append(image[y1:y2, x1:x2].copy())
            # update x1 and y2 later
            x1 += orb_w
        y1 += orb_h
        # reset x1 and x2
        x1 = x2 = initial
    return orbs

# %%
# detect the colour of all orbs and return a list
def detectColour(orbs):
    output = ""

    # first do colour detection
    for i in range(len(orbs)):
        curr = "?"
        src = orbs[i]
        # resize to be larger than the template
        src = cv.resize(src, orb_size)

        # cv.imshow("orb", src)
        # cv.waitKey()

        # match template first because jammer can be recognised as water
        gray = cv.cvtColor(src, cv.COLOR_BGR2GRAY)
        for c in orb_templates:
            curr_template = orb_templates[c][0]
            m = cv.matchTemplate(gray, curr_template, cv.TM_CCOEFF_NORMED)
            threshold = orb_templates[c][1]
            match = False
            for pt in zip(*np.where(m >= threshold)[::-1]):
                match = True
                break
            if match:
                curr = c
                break
        
        scale = 0.6
        x, _ = orb_size
        # Calculate new size and offset for both x and y
        size = int(x * scale)
        start = int((x - size) / 2)
        end = size + start
        # get the zoomed orb
        zoomed = src[start:end, start:end]
        # cv.imshow("zoomed", zoomed)
        # cv.waitKey()

        # match colour if no template matches
        if curr == "?":
            # convert to hsv
            hsv = cv.cvtColor(zoomed, cv.COLOR_BGR2HSV)
            # check which colour matches
            for c in colour_range:
                curr_range = colour_range[c]
                mask = cv.inRange(hsv, curr_range[0], curr_range[1])
                contours, _ = cv.findContours(mask, cv.RETR_CCOMP, cv.CHAIN_APPROX_SIMPLE)
                # get the sum of all mask areas
                area = sum(map(lambda c: cv.contourArea(c), contours))
                min_area = 800
                if c == "H":
                    min_area = 300
                # Usually it needs to be at least about 1000 because of the plus (for recovery about 400), usually it is around 9000
                if area > min_area:
                    curr = c
                    break
        output += curr
    return output

# %%
# sort all location for jammer and bomb
def sortLocation(a, b):
    # 140, 141, 142 are treated as one point here
    if abs(a[0] - b[0]) < match_offset:
        return a[1] - b[1]
    return a[0] - b[0]

# %%
def run():
    # take a screenshot at board_loc, need to subtract because it is width and height
    left, top, end_left, end_top = board_loc
    width = (end_left - left) * screen_scale
    height = (end_top - top) * screen_scale


    box = (left * screen_scale, top * screen_scale, left * screen_scale+width, top * screen_scale+height)
    #image.screenShot('').subImage(box).writeToFile(thispath, "board")
    screen = G.DEVICE.snapshot()
    screen = aircv.crop_image(screen,box)
    pil_img = cv2_2_pil(screen)
    pil_img.save("board.png", quality=99, optimize=True)

    #board_img = gui.screenshot(region=(left * screen_scale, top * screen_scale, width, height))
    # save the img for open cv
    #board_img.save("./board.png")

    # scan through the entire image
    src = cv.imread(thispath+"\\board.png")
    # src = cv.imread("sample/battle.png")

    # resize it to about 830, 690 which is the size I use
    src = cv.resize(src, board_size)
    # # add some padding in case orbs are too close to the border
    # src = cv.copyMakeBorder(src, border_len, border_len, border_len, border_len, cv.BORDER_CONSTANT)

    # # convert to grayscale
    # gray = cv.cvtColor(src, cv.COLOR_BGR2GRAY)
    # # create a binary thresholded image
    # _, binary = cv.threshold(gray, 80, 255, cv.THRESH_BINARY_INV)
    # # find the contours from the thresholded image
    # contours, _ = cv.findContours(binary, cv.RETR_CCOMP, cv.CHAIN_APPROX_SIMPLE)
    # # for c in contours: print(cv.contourArea(c))
    # # Only orbs and poison
    # orbContours = [c for c in contours if 10000 < cv.contourArea(c) < 20000]

    # # match jammer and bomb
    # jammer_len = bomb_len = 0
    # """
    # Jammer detection
    # """
    # curr_pt = None
    # w, h = jammer_template.shape[::-1]
    # jm = cv.matchTemplate(gray, jammer_template, cv.TM_CCORR_NORMED)
    # threshold = 0.9
    # jammer_loc = sorted(zip(*np.where(jm >= threshold)[::-1]), key=cmp_to_key(sortLocation))
    # for pt in jammer_loc:
    #     # remove duplicates
    #     if curr_pt is None:
    #         curr_pt = pt
    #     elif abs(curr_pt[0] - pt[0]) < match_offset and abs(curr_pt[1] - pt[1]) < match_offset:
    #             continue
    #     else:
    #         curr_pt = pt
    #     # print(pt)
    #     # count and draw a box
    #     jammer_len += 1
    #     # cv.rectangle(src, pt, (pt[0] + w, pt[1] + h), (0, 0, 255), 1)
    # print("There are {} jammer(s)".format(jammer_len))
    # """
    # Bomb detection
    # """
    # curr_pt = None
    # w, h = bomb_template.shape[::-1]
    # bm = cv.matchTemplate(gray, bomb_template, cv.TM_CCOEFF_NORMED)
    # threshold = 0.515
    # bomb_loc = sorted(zip(*np.where(bm >= threshold)[::-1]), key=cmp_to_key(sortLocation))
    # for pt in bomb_loc:
    #     # remove duplicates
    #     if curr_pt is None:
    #         curr_pt = pt
    #     elif abs(curr_pt[0] - pt[0]) < match_offset and abs(curr_pt[1] - pt[1]) < match_offset:
    #             continue
    #     else:
    #         curr_pt = pt
    #     # print(pt)
    #     # count and draw a box        
    #     bomb_len += 1
    #     # cv.rectangle(src, pt, (pt[0] + w, pt[1] + h), (0, 255, 0), 1)
    # print("There are {} bomb(s)".format(bomb_len))

    # # get every single orb here
    # orb_count = len(orbContours) + jammer_len + bomb_len
    # print("There are {} orbs in total".format(orb_count))
    orbs = getEachOrb(src)
    # cv.imshow("orb", orbs[3])

    # detect the colour of every orb and output a string, the list is in order so simple join the list will do
    output = detectColour(orbs)

    # draw all contours
    # image = cv.drawContours(src, orbContours, -1, (255, 255, 255), 3)
    # cv.imshow("board", src)
    # cv.imshow("bw", binary)
    # cv.waitKey()

    return output

# %%
# 0,0 0,1 0,2 0,3 -> 0,0 0,3
def shorten0(path):
    # copy the original list
    temp = path[:]
    prev = after = None
    l = len(path)
    count = 0
    for index, item in enumerate(path):
        if index > 0 and index < (l - 1):
            prev = path[index - 1]
            after = path[index + 1]
            # remove item if it is in the middle
            if (prev[0] == item[0] == after[0] or prev[1] == item[1] == after[1]):
                temp.remove(item)
                # make prev longer to move
                prev.append(0.5)
                count += 1
    print("Shortened {} moves".format(count)) 
    return temp

# %%
def perform(solution):
    print("- PERFORMING -")
    # setup everything
    left, top, end_left, end_top = board_loc
    orb_height = (end_top - top) / board_row
    x_start = left + orb_height / 2
    y_start = top + orb_height / 2

    # save current position
    #(px, py) = gui.position()
    step = len(solution)
    print("{} steps in total.".format(step))
    start = time.time()
    fullstep = []
    for i in range(step):
        curr = solution[i]
        x, y = curr
        target_x = x_start + y * orb_height
        target_y = y_start + x * orb_height
        if i == 0:
            # need to hold the mouse
            #gui.mouseDown(target_x, target_y, button='left')
            #gui.mouseDown(target_x, target_y, button='left')  
            fullstep.append([int(target_x), int(target_y)])   
      
        else:
            duration = 0.1
            if len(curr) == 3:
                duration = 0.2
            # simply move to there
            fullstep.append([int(target_x), int(target_y)])
            #gui.moveTo(target_x, target_y, duration=duration)
    #print(fullstep)
    dev.swipe_along(fullstep,duration=duration)
    print("Performed!")
    print("It took %.2fs." % (time.time() - start))
    # only release it when everything are all done
    #gui.mouseUp()
    # move back to current position after everything
    #gui.moveTo(px, py)

    # save solution image
    # width = (end_left - left) * screen_scale
    # height = (end_top - top) * screen_scale
    # board_img = gui.screenshot(region=(left * screen_scale, top * screen_scale, width, height))
    # board_img.save("./solution.png")
    width = (end_left - left) * screen_scale
    height = (end_top - top) * screen_scale
    box = (left * screen_scale, top * screen_scale, left * screen_scale+width, top * screen_scale+height)
    #image.screenShot('').subImage(box).writeToFile(thispath, "solution")
    screen = G.DEVICE.snapshot()
    screen = aircv.crop_image(screen,box)
    pil_img = cv2_2_pil(screen)
    pil_img.save("solution.png", quality=99, optimize=True)


# %%
def getSolution(input):
    print("- SOLVING -")
    if os.path.exists(thispath+"\\path.pazusoba"):
        os.remove(thispath+"\\path.pazusoba")
    start = time.time()
    solution = []
    completed = False
    failed_count = 0
    # make sure a solution is written to the disk
    while not completed:
        pazusoba = subprocess.Popen([thispath+'\\pazusoba.exe', input, '3', '40', '10000',' > log.txt'])
        pazusoba.wait()

        output_file = thispath+"\\path.pazusoba"
        if os.path.exists(output_file):
            with open(output_file, 'r') as p:
                solution = p.read().split('|')[:-1]
            # convert to int list
            solution = list(map(lambda x: list(map(int, x.split(','))), solution))
            completed = True
            print("Solved!")
        else:
            failed_count += 1
            print("Failed. Retry... {}".format(failed_count))
            if failed_count > 10:
                print("Failed. Try again later.")
                break
    print("It took %.2fs." % (time.time() - start))

    return shorten(solution)
    #return solution

# %%
def shorten(solution):
    print("- SIMPLIFYING -")
    # insert the first
    simplified_solution = [solution[0]]
    length = len(solution)

    # loop from the second step
    for i in range(1, length - 1):
        x0 = solution[i][0] - solution[i - 1][0]
        x1 = solution[i + 1][0] - solution[i][0]
        if x0 == x1:
            y0 = solution[i][1] - solution[i - 1][1]
            y1 = solution[i + 1][1] - solution[i][1]
            if y0 == y1:
                continue
        simplified_solution.append(solution[i])

    # insert the last
    simplified_solution.append(solution[length - 1])
    print("{} steps -> {} steps".format(length, len(simplified_solution)))
    return simplified_solution



def doit():
    # %%
    board = run()
    #print(board)
    # board = "RHHBDRDRGHDLLBGRRBRHBGGBHBDDHH"
    if "?" in board:
        print("Adjustment needed.")
        # print it out for me to see and manually adjust it above
        for i in range(board_row):
            start = i * board_column
            print(board[start:(start + board_column)])
    else:
        solution = getSolution(board)
        perform(solution)



def waitUntilDisplay(pic, sec=5):
    snapshot(filename="666.jpg",quality=99)
    origin = r"666.jpg"
    a = Phone()
    count = 0
    while not a.exist(pic, origin):
        time.sleep(0.5)
        snapshot(filename="666.jpg",quality=99)
        count += 1
        if count > (sec):
            return False
    else:
        print('until find pic!')
        loc = a.findpic(pic, origin)
        return loc


def waitToClick(pic ,time=5):
    loc = waitUntilDisplay(pic, time)
    try:
        if loc is not False:
            swipe(loc,loc,duration=0)
            #touch([580,1000])
            #touch(loc)
        else:
            print('cannot find pic !! please confirm')
    except Exception as e:
        print(e)






# 20, 30 or 42
orb_count = 30

def levelup():
    while waitUntilDisplay('ok.jpg',sec=2):
        waitToClick('ok.jpg')
    waitToClick('allfb.jpg')
    if not waitUntilDisplay('jishu1.jpg',sec=2):
        waitToClick('jishu.jpg')
    swipe([547, 671],[547, 671],duration=0)  # 第一个本
    time.sleep(2)
    swipe([547, 671],[547, 671],duration=0)  # 第一个小本
    while not waitUntilDisplay("friend.jpg",sec=2):
        swipe([547, 671],[547, 671],duration=0)  # 第一个小本
    waitToClick("friend.jpg")
    waitToClick("change.jpg")    
    time.sleep(5)
    while not waitUntilDisplay('battle.jpg'):
        time.sleep(2)

    while not waitUntilDisplay("ok.jpg"):
        doit()
        time.sleep(20)

    waitToClick("ok.jpg")
    time.sleep(8)
    while not waitUntilDisplay("result.jpg"):
        swipe([29, 1025],[29, 1025],duration=0)
        swipe([29, 1025],[29, 1025],duration=0)
        swipe([29, 1025],[29, 1025],duration=0)
        swipe([29, 1025],[29, 1025],duration=0)
        swipe([29, 1025],[29, 1025],duration=0)
    while waitUntilDisplay("result.jpg"):    
        swipe([29, 1025],[29, 1025],duration=0)
        swipe([29, 1025],[29, 1025],duration=0)
        swipe([29, 1025],[29, 1025],duration=0)
        swipe([29, 1025],[29, 1025],duration=0)
        swipe([29, 1025],[29, 1025],duration=0)


def event():
    while True:
        while waitUntilDisplay('ok.jpg',sec=2):
            waitToClick('ok.jpg')
        waitToClick('allfb.jpg')
        waitToClick('eventfb.jpg')
        swipe([29, 1025],[29, 1625],duration=0.5)
        time.sleep(1)
        swipe([29, 1025],[29, 1625],duration=0.5)
        while not waitUntilDisplay("event.jpg"):
            swipe([29, 1025],[29, 625],duration=0.5)
        waitToClick("event.jpg")  
        swipe([29, 1025],[29, 1625],duration=0.5)
        time.sleep(1)
        swipe([29, 1025],[29, 1625],duration=0.5)
        while not waitUntilDisplay("event1.jpg"):
            swipe([29, 1025],[29, 625],duration=0.5)       
        waitToClick("event1.jpg")  
        if waitUntilDisplay("back.jpg"):
            waitToClick("back.jpg")  
            break
        else:
            waitToClick("friend.jpg")
            waitToClick("change.jpg")    
            time.sleep(5)
            while not waitUntilDisplay('battle.jpg'):
                time.sleep(2)

            while not waitUntilDisplay("ok.jpg"):
                doit()
                time.sleep(20)

            waitToClick("ok.jpg")
            time.sleep(8)
            while not waitUntilDisplay("result.jpg"):
                swipe([29, 1025],[29, 1025],duration=0)
                swipe([29, 1025],[29, 1025],duration=0)
                swipe([29, 1025],[29, 1025],duration=0)
                swipe([29, 1025],[29, 1025],duration=0)
                swipe([29, 1025],[29, 1025],duration=0)
            while waitUntilDisplay("result.jpg"):    
                swipe([29, 1025],[29, 1025],duration=0)
                swipe([29, 1025],[29, 1025],duration=0)
                swipe([29, 1025],[29, 1025],duration=0)
                swipe([29, 1025],[29, 1025],duration=0)
                swipe([29, 1025],[29, 1025],duration=0)

while True:
    event()
    levelup()
    #doit()
    #snapshot(filename="123.jpg",msg="首页截图",quality=99)
    # if not waitUntilDisplay('result.jpg',sec=2):
    #     waitToClick('fb.jpg')
    #     time.sleep(5)
    #     swipe([580, 1000],[580, 1000],duration=0)
    #     waitToClick("friend.jpg")
    #     waitToClick("change.jpg")

    #     import time
    #     time.sleep(5)
    #     while not waitUntilDisplay('battle.jpg'):
    #         time.sleep(2)

    #     while not waitUntilDisplay("ok.jpg"):
    #         doit()
    #         time.sleep(20)

    #     waitToClick("ok.jpg")
    #     time.sleep(8)
    #     while waitUntilDisplay("result.jpg"):
    #         time.sleep(3)
    #         swipe([29, 1025],[29, 1025],duration=0)
    #         swipe([29, 1025],[29, 1025],duration=0)
    #         swipe([29, 1025],[29, 1025],duration=0)


