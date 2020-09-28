import matplotlib.pyplot as plotter
packets = 10
request = 6
response = 4


def create_graph():
    pieLabels = 'Request', 'Response'
    populationShare = [(request/packets)*100, (response/packets)*100]
    my_colors = ['lightgreen', 'lightblue']
    figureObject, axesObject = plotter.subplots()
    axesObject.set_title('Packets Received')
    axesObject.pie(populationShare,
                   # The fields of the graph
                   labels=pieLabels,
                   # two points after the decimal
                   autopct='%1.2f',
                   startangle=90,
                   # colors for the graph
                   colors=my_colors
                   )

    # Aspect ratio - equal means pie is a circle
    axesObject.axis('equal')
    plotter.show()


def main():
    create_graph()


if __name__ == '__main__':
    main()