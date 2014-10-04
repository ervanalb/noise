if __name__ == "__main__":
    wb = WaveBlock()
    ui = UIBlock()
    wb.set_input(0, ui, 0)
    ui.set_input(0, wb, 0)

    while True:
        result = ui.pull()
        data=struct.pack('f'*c.CHUNKSIZE,*(ui.output[:c.CHUNKSIZE]))
        print ui.output[:c.CHUNKSIZE]
