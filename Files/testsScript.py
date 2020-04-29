import stdnum.iso6346
import random
import os


def make():
    l = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
    x = ''
    for i in range(3):
        x += random.choice(l)
    x += random.choice('UJZ')
    for i in range(6):
        x += random.choice('0123456789')
    x += stdnum.iso6346.calc_check_digit(x)
    return x


def makeTest(name, num_travels, num_ports, num_containers):
    root_path = os.path.join(os.getcwd(), name)
    os.mkdir(root_path)
    travels = ["travel" + str(i + 1) for i in range(num_travels)]
    for travel in travels:
        path = os.path.join(root_path, travel)
        os.mkdir(path)
        plan = open(os.path.join(path, 'Plan.csv'), 'w')
        floors = random.randint(2, 6)
        row = random.randint(2, 5)
        col = random.randint(2, 5)
        plan.write(str(floors) + ',' + str(row) + ',' + str(col) + '\n')
        for i in range(row - 1):
            plan.write(str(random.randint(0, row - 1)) + ',' + str(random.randint(0, col - 1)) + ',' + str(
                random.randint(1, floors)) + '\n')
        plan.close()
        semi1 = [chr(ord('A') + i) * 2 for i in range(26)]
        semi2 = [chr(ord('A') + i) * 3 for i in range(26)]
        L = [i + j for i, j in zip(semi1, semi2)]
        ports = []
        for i in range(num_ports):
            ports.append(random.choice(L))
        route = open(os.path.join(path, 'Route.csv'), 'w')
        for i in range(num_ports):
            route.write(ports[i] + '\n')
        route.close()
        portsFiles = []
        ports_num_visits = {}
        for i in range(num_ports - 1):
            ports_num_visits[ports[i]] = 1
        for i in range(num_ports - 1):
            t = open(os.path.join(path, ports[i]) + '_' + str(ports_num_visits[ports[i]]) + '.cargo_data.csv', 'w')
            portsFiles.append(os.path.join(path, ports[i]) + '_' + str(ports_num_visits[ports[i]]) + '.cargo_data.csv')
            ports_num_visits[ports[i]] += 1
            t.close()
        for i in range(num_containers):
            name = make()
            port = random.choice(portsFiles)
            t = open(port, 'a')
            t.write(name + ',' + str(random.randint(1, 10)) + ',' + random.choice(
                ports[portsFiles.index(port) + 1:]) + '\n')
            t.close()


makeTest('tests', 20, 6, 15)
