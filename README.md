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
  -m ~/models/qwen3/Qwen3-1.7B-Q4_K_M.gguf \
  -t 4 \
  -c 512
```

Server runs at:

http://localhost:8080

---

## Build qcli

```bash
gcc qcli.c -o qcli -lcurl
```

---

## Run qcli

```bash
./qcli "Explain quicksort"
```

---

## Example Workflow

1. Start server
```bash
./llama-server -m model.gguf -t 4 -c 512
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
- No streaming output yet
- Basic JSON parsing
- Experimental project

---

## Future Improvements

- Streaming responses
- Interactive REPL
- History support
- Raw socket HTTP implementation
- Config file support

---

## License

MIT
