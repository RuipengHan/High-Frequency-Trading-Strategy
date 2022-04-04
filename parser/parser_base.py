'''
This is the base for our parser.
'''

class ParserBase:
    '''
    All data parsers should inherit this format.
    Implement download function to download raw data.
    Implement parse function to convert raw data into SS format.
    '''
    def __init__(self) -> None:
        pass

    def download(self, time_start, time_end, dest):
        '''
        A function that obtains raw data from certain source.
        Args:
            time_start: start timestamp to obtain data
            time_end: end timestamp to obtain data
            dest: destination to store data
        '''
        raise NotImplementedError("download function not implemented")

    def parse(self, source, dest):
        '''
        A function that converts raw data to Strategy Studio recongnized format.
        Args:
            source: raw data file
            dest: destination data file
        '''
        raise NotImplementedError("parse function not implemented")
