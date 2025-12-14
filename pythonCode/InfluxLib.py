from urllib import request, parse, error

INFLUX_BASE_URL = "http://192.168.69.32:8086"
ORG = "VAMK"
BUCKET = "BMS"
TOKEN = "QnXi3FDHQXeK9ab7AXUlD0VqzQUOeDM3z3N_igHigSwHW9MpBqnAVI_SptWBW1lT55x2x4t5cxMKmI4rArxnTQ=="


def _make_write_url() -> str:
    base = INFLUX_BASE_URL.rstrip("/") + "/api/v2/write"
    query = {
        "org": ORG,
        "bucket": BUCKET,
        "precision": "s",
    }
    return base + "?" + parse.urlencode(query)


WRITE_URL = _make_write_url()


class InfluxLib:
 

    def __init__(self):
        print(f"[InfluxLib] Using write URL: {WRITE_URL}", flush=True)

    def _post_line(self, line: str):
        data = line.encode("utf-8")

        req = request.Request(WRITE_URL, data=data, method="POST")
        req.add_header("Authorization", f"Token {TOKEN}")
        req.add_header("Content-Type", "text/plain; charset=utf-8")

        try:
            with request.urlopen(req) as resp:
                status = resp.getcode()
                if status != 204:
                    body = resp.read().decode("utf-8", errors="ignore")
                    raise AssertionError(f"Influx write failed: {status} {body}")
        except error.HTTPError as e:
            body = e.read().decode("utf-8", errors="ignore")
            raise AssertionError(f"Influx HTTPError: {e.code} {body}")
        except Exception as e:
            raise AssertionError(f"Influx write error: {e}")

    def write_temp_to_influx(self, value):
  
        value = float(value)
        line = f"TEMP_measurement,host=debian value={value}"
        print(f"[InfluxLib] Sending: {line}", flush=True)
        self._post_line(line)

    def write_current_to_influx(self, value):
   
        value = float(value)
        line = f"CURRENT_measurement,host=debian value={value}"
        print(f"[InfluxLib] Sending: {line}", flush=True)
        self._post_line(line)
