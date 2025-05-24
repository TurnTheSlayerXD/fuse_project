import requests as req

def main():
    url = 'https://api.telegram.org/bot6947966209:AAGuVGoVPuU-KK3QjvY-3lr_D3zQgujRwyo/sendDocument?chat_id=-1002556273060'
    
    with open ('./test.txt', 'rb') as f:
        request = req.Request('POST', url, files={'document' : f})
        prepared = request.prepare()
        pretty_print_POST(prepared)

def pretty_print_POST(request):
    """
    At this point it is completely built and ready
    to be fired; it is "prepared".

    However pay attention at the formatting used in 
    this function because it is programmed to be pretty 
    printed and may differ from the actual request.
    """
    print('{}\n{}\r\n{}\r\n\r\nBody:\r\n{}'.format(
        '-----------START-----------',
        request.method + ' ' + request.url,
        '\r\n'.join('{}: {}'.format(k, v) for k, v in request.headers.items()),
        request.body,
        
    ))



if __name__ == '__main__':
    main()
    pass