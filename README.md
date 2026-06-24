# qcli

A minimal Unix-style CLI for interacting with a local LLM server (llama.cpp / Qwen models).

qcli sends prompts to a local llama.cpp server and prints clean responses in the terminal, without heavy dependencies or frameworks.

---

## Features

- Minimal C implementation
- Only dependency: libcurl
- Works with llama.cpp server
- Clean terminal output
- Scriptable (pipe-friendly)
- Fast and lightweight

---

## Requirements

Install build tools and dependencies:

```bash
sudo apt update
sudo apt install build-essential libcurl4-openssl-dev
```

You also need a working build of llama.cpp.

---

## Run the LLM Server (llama.cpp)

Start the local model server:

```bash
./build/bin/llama-server \
  -m ../models/qwen3/Qwen3-1.7B-Q4_K_M.gguf \
  -t 2 \
  -c 4096 \
  --parallel 1 \
```

Key flags:

- `-t 2` — physical core count (hyperthreads hurt inference on older CPUs; benchmark `-t 1`, `-t 2`, `-t 4`)
- `--parallel 1` — single request slot, avoids allocating 4x KV cache for unused slots
- `-fa off` — flash attention, reduces memory bandwidth pressure

Server runs at:

http://localhost:8080

---

## Build qcli

```bash
make
```

To remove the binary:

```bash
make clean
```

---

## Run qcli

```bash
./qcli "Explain quicksort"
```

![qcli output](assets/images/Screenshot%20from%202026-06-24%2013-13-57.png)

---

## Example Workflow

1. Start server

```bash
./llama-server -m model.gguf -t 2 -c 512 --parallel 1 -fa
```

2. Run CLI

```bash
./qcli "What is recursion?"
```

---

## Design Philosophy

- Do one thing well
- No frameworks
- Minimal dependencies
- Easy to hack and modify
- UNIX-style simplicity

---

## Limitations

- Requires running llama.cpp server
- Basic JSON parsing (SSE)
- Experimental project

---

## Future Improvements

- Interactive REPL
- History support
- Raw socket HTTP implementation
- Config file support

---

## Tested Hardware

Verified working on:

- **CPU:** Intel Core i5-2450M @ 2.50GHz (2 physical cores / 4 threads, Sandy Bridge)
- **RAM:** 8 GB
- **GPU:** None (CPU-only inference)
- **OS:** Linux
- **Model:** Qwen3-1.7B-Q4_K_M.gguf
- **Throughput:** ~6 t/s with the server flags above

---

## License

MIT
