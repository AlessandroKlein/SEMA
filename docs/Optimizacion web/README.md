
## Minificar el HTML

### Instala el minificador:

```bash
npm install -g html-minifier
```

### Minifica tu archivo:

  - Windows:
    ```bash
    Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope Process
    ```
    ```bash
    npx html-minifier --collapse-whitespace --remove-comments --minify-js true --minify-css true index.html -o index.min.html
    ```

```bash
npx html-minifier --collapse-whitespace --remove-comments --minify-js true --minify-css true index.html -o index.min.html
```
```bash
npx html-minifier ^
  --collapse-whitespace ^
  --remove-comments ^
  --remove-redundant-attributes ^
  --remove-script-type-attributes ^
  --remove-style-link-type-attributes ^
  --use-short-doctype ^
  --minify-css true ^
  --minify-js true ^
  --sort-attributes ^
  --sort-class-name ^
  --decode-entities ^
  --remove-empty-attributes ^
  --remove-optional-tags ^
  index.html -o index.min.html
```
```bash
npx html-minifier --collapse-whitespace --remove-comments --remove-redundant-attributes --remove-script-type-attributes --remove-style-link-type-attributes --use-short-doctype --minify-css true --minify-js true --sort-attributes --sort-class-name --decode-entities --remove-empty-attributes --remove-optional-tags index.html -o index.min.html
```

> Si estás en PowerShell, reemplaza `^` por `\` o todo en una sola línea.

🔍 ¿Qué hacen estas opciones?

| Opción                                | Descripción breve                                  |
| ------------------------------------- | -------------------------------------------------- |
| `--collapse-whitespace`               | Elimina espacios innecesarios                      |
| `--remove-comments`                   | Quita comentarios HTML                             |
| `--remove-redundant-attributes`       | Elimina atributos redundantes (`type="text"` etc.) |
| `--remove-script-type-attributes`     | Limpia `type="text/javascript"`                    |
| `--remove-style-link-type-attributes` | Limpia `type="text/css"`                           |
| `--use-short-doctype`                 | Usa `<!doctype html>` en vez de largo              |
| `--minify-css true`                   | Minifica contenido CSS inline                      |
| `--minify-js true`                    | Minifica JavaScript inline                         |
| `--sort-attributes`                   | Ordena atributos alfabéticamente                   |
| `--sort-class-name`                   | Ordena clases por nombre                           |
| `--decode-entities`                   | Decodifica entidades HTML (`&amp;` → `&`)          |
| `--remove-empty-attributes`           | Elimina atributos vacíos (`id=""`)                 |
| `--remove-optional-tags`              | Elimina etiquetas como `</li>` innecesarias        |

___

### Ofuscar JavaScript

Usa [`javascript-obfuscator`](https://github.com/javascript-obfuscator/javascript-obfuscator):

```bash
npm install -g javascript-obfuscator
```

Ofuscar archivo JS:

```bash
javascript-obfuscator script.js --output script.obf.js
```

Puedes luego insertar el `script.obf.js` en tu HTML minificado.

___

Ofuscar CSS
___
## Comprimir el HTML con Gzip

### En Linux/macOS:

```bash
gzip -k -9 index.min.html
```

### En Windows:

```bash
cd "/d/Users/alekl/Documents/Arduino/Base Web/Invernadero/min"
```
```bash
gzip -k -9 index.min.html
```
```bash
xxd -i index.min.html.gz > index_html.h
```
___

### CSS JS, HTML Proceso recomendado (paso a paso)

1. **Ofuscar o minificar JS**
   `javascript-obfuscator script.js -o script.obf.js`

2. **Minificar u optimizar CSS**
   `cleancss -O2 style.css -o style.min.css`

3. **Minificar HTML con enlaces a los archivos anteriores:**

```bash
html-minifier ^
  --collapse-whitespace ^
  --remove-comments ^
  --minify-css true ^
  --minify-js true ^
  --remove-redundant-attributes ^
  --remove-script-type-attributes ^
  --remove-style-link-type-attributes ^
  --remove-empty-attributes ^
  --decode-entities ^
  index.html -o index.min.html
```

___

### Sugerencia final: Automatiza el proceso

Puedes crear un script `.bat` en Windows como este:

```bat
@echo off
echo === Ofuscando JS ===
javascript-obfuscator script.js -o script.obf.js

echo === Minificando CSS ===
cleancss -O2 style.css -o style.min.css

echo === Minificando HTML ===
html-minifier --collapse-whitespace --remove-comments --minify-js true --minify-css true index.html -o index.min.html

echo === Listo ===
pause
```
___
## Convertir archivo .gz a .h

### Opción 1: Usar xxd (Linux/macOS o Git Bash en Windows):
```bash
xxd -i index.min.html.gz > index_html.h
```
Esto generará un archivo con contenido como:
```c
unsigned char index_min_html_gz[] = {
  0x1f, 0x8b, 0x08, 0x08, ...
};
unsigned int index_min_html_gz_len = 1024;
```

### Opción 2: Herramienta en línea
Puedes usar:
https://tomeko.net/online_tools/file_to_hex.php?lang=en
(elige "C array")

___
## Incluir en el sketch de ESP32 (sin SPIFFS)

### Incluye el archivo .h:

```c
#include "index_html.h"
```

### Sirve la página desde memoria flash:
Usando ESPAsyncWebServer:

```c
server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  AsyncWebServerResponse *response = request->beginResponse_P(
    "text/html", index_min_html_gz_len, index_min_html_gz);
  response->addHeader("Content-Encoding", "gzip");
  request->send(response);
});
```
___
### Opcion 3: con python y archivo "data.hex"
```python
with open("data.hex", "r") as f:
    content = f.read()

# Limpiar contenido y contar elementos
bytes_list = [b.strip() for b in content.split(",") if "0x" in b]
print(f"Número total de bytes: {len(bytes_list)}")
```
