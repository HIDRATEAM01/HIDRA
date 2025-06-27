export function parseDateString(dateString) {
  const [dia, mes, ano] = dateString.split("/");
  if (!dia || !mes || !ano) {
    throw new Error("Invalid date format");
  }
  const date = new Date(`${ano}-${mes}-${dia}`);
  return date;
}

export function parseTimeString(timeString) {
  const [horas, minutos, segundos] = timeString.split(":");
  if (!horas || !minutos || !segundos) {
    throw new Error("Invalid time format");
  }
  const date = new Date();
  date.setHours(
    parseInt(horas, 10),
    parseInt(minutos, 10),
    parseInt(segundos, 10)
  );
  return date;
}

export function parseToDateTime(dateString, timeString) {
  const date = parseDateString(dateString);
  const time = parseTimeString(timeString);
  if (!date || !time) {
    return null;
  }
  return new Date(
    date.getFullYear(),
    date.getMonth(),
    date.getDate(),
    time.getHours(),
    time.getMinutes()
  );
}
