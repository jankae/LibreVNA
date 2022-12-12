import socket
from asyncio import IncompleteReadError  # only import the exception class
import time

class SocketStreamReader:
    def __init__(self, sock: socket.socket):
        self._sock = sock
        self._recv_buffer = bytearray()
        self.timeout = 1.0

    def read(self, num_bytes: int = -1) -> bytes:
        raise NotImplementedError

    def readexactly(self, num_bytes: int) -> bytes:
        buf = bytearray(num_bytes)
        pos = 0
        while pos < num_bytes:
            n = self._recv_into(memoryview(buf)[pos:])
            if n == 0:
                raise IncompleteReadError(bytes(buf[:pos]), num_bytes)
            pos += n
        return bytes(buf)

    def readline(self) -> bytes:
        return self.readuntil(b"\n")

    def readuntil(self, separator: bytes = b"\n") -> bytes:
        if len(separator) != 1:
            raise ValueError("Only separators of length 1 are supported.")

        chunk = bytearray(4096)
        start = 0
        buf = bytearray(len(self._recv_buffer))
        bytes_read = self._recv_into(memoryview(buf))
        assert bytes_read == len(buf)

        timeout = time.time() + self.timeout
        while True:
            idx = buf.find(separator, start)
            if idx != -1:
                break
            elif time.time() > timeout:
                raise Exception("Timed out waiting for response from GUI")

            start = len(self._recv_buffer)
            bytes_read = self._recv_into(memoryview(chunk))
            buf += memoryview(chunk)[:bytes_read]

        result = bytes(buf[: idx + 1])
        self._recv_buffer = b"".join(
            (memoryview(buf)[idx + 1 :], self._recv_buffer)
        )
        return result

    def _recv_into(self, view: memoryview) -> int:
        bytes_read = min(len(view), len(self._recv_buffer))
        view[:bytes_read] = self._recv_buffer[:bytes_read]
        self._recv_buffer = self._recv_buffer[bytes_read:]
        if bytes_read == len(view):
            return bytes_read
        try:
            bytes_read += self._sock.recv_into(view[bytes_read:], 0, socket.MSG_DONTWAIT)
        except:
            pass
        return bytes_read

class libreVNA:
    def __init__(self, host='localhost', port=19542):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            self.sock.connect((host, port))
        except:
            raise Exception("Unable to connect to LibreVNA-GUI. Make sure it is running and the TCP server is enabled.")
        self.reader = SocketStreamReader(self.sock)

    def __del__(self):
        self.sock.close()

    def __read_response(self):
        return self.reader.readline().decode().rstrip()

    def cmd(self, cmd):
        self.sock.sendall(cmd.encode())
        self.sock.send(b"\n")
        resp = self.__read_response()
        if len(resp) > 0:
        	raise Exception("Expected empty response but got "+resp)

    def query(self, query):
        self.sock.sendall(query.encode())
        self.sock.send(b"\n")
        return self.__read_response()
    
    @staticmethod
    def parse_VNA_trace_data(data):
        ret = []
        # Remove brackets (order of data implicitly known)
        data = data.replace(']','').replace('[','')
        values = data.split(',')
        if int(len(values) / 3) * 3 != len(values):
            # number of values must be a multiple of three (frequency, real, imaginary)
            raise Exception("Invalid input data: expected tuples of three values each")
        for i in range(0, len(values), 3):
            freq = float(values[i])
            real = float(values[i+1])
            imag = float(values[i+2])
            ret.append((freq, complex(real, imag)))
        return ret
    
    @staticmethod
    def parse_SA_trace_data(data):
        ret = []
        # Remove brackets (order of data implicitly known)
        data = data.replace(']','').replace('[','')
        values = data.split(',')
        if int(len(values) / 2) * 2 != len(values):
            # number of values must be a multiple of two (frequency, dBm)
            raise Exception("Invalid input data: expected tuples of two values each")
        for i in range(0, len(values), 2):
            freq = float(values[i])
            dBm = float(values[i+1])
            ret.append((freq, dBm))
        return ret

