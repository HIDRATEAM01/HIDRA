function newClock() {
  return new Date("2000-01-01T00:00:00");
}

export function parseDateString(dateString) {
  if (dateString === null) return null;
  let [ano, mes, dia] = [null, null, null];

  if (dateString.indexOf("/") === -1) {
    [ano, mes, dia] = dateString.split("-");
  } else {
    [dia, mes, ano] = dateString.split("/");
  }
  if (!dia || !mes || !ano) {
    return null;
  }
  const date = new Date(`${ano}-${mes}-${dia}`);
  return date;
}

export function parseTimeString(timeString) {
  if (timeString === null) return null;
  const [horas, minutos, segundos] = timeString.split(":");
  if (!horas || !minutos || !segundos) {
    return null;
  }
  const date = newClock();
  date.setHours(
    parseInt(horas, 10),
    parseInt(minutos, 10),
    parseInt(segundos, 10)
  );
  return date;
}

export function getDateString(clock) {
  if (!clock) clock = newClock();
  const year = clock.getFullYear();
  const month = String(clock.getMonth() + 1).padStart(2, "0");
  const day = String(clock.getDate()).padStart(2, "0");
  return `${year}-${month}-${day}`;
}

export function getTimeString(clock) {
  if (!clock) clock = newClock();
  const hours = String(clock.getHours()).padStart(2, "0");
  const minutes = String(clock.getMinutes()).padStart(2, "0");
  const seconds = String(clock.getSeconds()).padStart(2, "0");
  return `${hours}:${minutes}:${seconds}`;
}

export function updateClock(oldClock, dateString, timeString) {
  const date = parseDateString(dateString);
  const time = parseTimeString(timeString);

  if (!oldClock) {
    oldClock = newClock();
  }

  let clock = new Date(oldClock.getTime());

  if (date) {
    clock.setFullYear(date.getFullYear());
    clock.setMonth(date.getMonth());
    clock.setDate(date.getDate());
  }
  if (time) {
    clock.setHours(time.getHours());
    clock.setMinutes(time.getMinutes());
    clock.setSeconds(time.getSeconds());
  }
  return clock;
}
