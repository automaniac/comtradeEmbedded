static const char PgWeb[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<body>

<p>Click on the "Choose File" button to upload a file:</p>

<form method="post" enctype="multipart/form-data">
  <label for="file">File</label>
  <input id="file" name="file" type="file" />
  <button>Upload</button>
</form>
</body>
</html>

)rawliteral";