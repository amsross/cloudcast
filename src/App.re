let excludes = [|`hourly, `minutely, `current, `alerts|];

let main = (lat, lon, key) => {
  let now = Js.Date.make()->Js.Date.valueOf->(/.)(1000.);

  OpenWeather.OneCall.get(lat, lon, excludes, key)
  ->Future.map(response =>
      switch (response) {
      | Ok({daily: Some(daily)}) => Ok(daily)
      | _ => Error("No daily response found")
      }
    )
  ->Future.mapOk(daily =>
      daily->Belt.Array.reduce(
        None,
        (prev, next) => {
          let prev = prev->Belt.Option.getWithDefault(next);

          prev.sunset < now || prev.sunset > next.sunset
            ? Some(next) : Some(prev);
        },
      )
    )
  ->Future.map(response =>
      switch (response) {
      | Ok(Some(daily)) => Ok(daily)
      | Error(_) as err => err
      | _ => Error("No daily response found")
      }
    )
  ->Future.map(response =>
      switch (response) {
      | Ok({clouds, pop, feels_like: {eve}} as daily) =>
        if (clouds > 25.) {
          Error("Too cloudy - " ++ Js.Float.toString(clouds) ++ "%");
        } else if (pop > 20.) {
          Error("Might Rain - " ++ Js.Float.toString(pop) ++ "%");
        } else if (eve < 20.) {
          Error("Too Cold - " ++ Js.Float.toString(eve));
        } else {
          Ok(daily);
        }
      | err => err
      }
    );
};
