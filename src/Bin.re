exception MissingEnv(string);

module Function = Helpers.Function;
module Option = Helpers.Option;
module Result = Helpers.Result;

[@bs.val] [@bs.scope ("process", "env")]
external key: option(Js.Json.t) = "API_KEY";
[@bs.val] [@bs.scope ("process", "env")]
external lat: option(Js.Json.t) = "LAT";
[@bs.val] [@bs.scope ("process", "env")]
external lon: option(Js.Json.t) = "LON";
[@bs.val] [@bs.scope ("process", "env")]
external phone_number: option(Js.Json.t) = "PHONE_NUMBER";

let (lat, lon, key, phone_number) =
  Json.Decode.(
    lat
    ->Option.map(either(float, map(float_of_string, string)))
    ->Option.toResult(MissingEnv("Missing LAT")),
    lon
    ->Option.map(either(float, map(float_of_string, string)))
    ->Option.toResult(MissingEnv("LON")),
    key
    ->Option.map(string)
    ->Option.toResult(MissingEnv("API_KEY")),
    phone_number
    ->Option.map(string)
    ->Option.toResult(MissingEnv("PHONE_NUMBER")),
  );

Future.value(Result.liftA3(App.main, lat, lon, key))
->Future.flatMapOk(Function.id)
->Future.get(result =>
    switch (result) {
    | Ok({sunset, clouds, pop, feels_like: {eve}}) =>
      let sunset =
        sunset->( *. )(1000.)->Js.Date.fromFloat->Js.Date.toLocaleTimeString;
      let clouds = clouds->Js.Float.toString;
      let pop = pop->Js.Float.toString;
      let eve = eve->Js.Float.toString;

      Js.Console.log(
        "sunset: "
        ++ sunset
        ++ "\nevening temp: "
        ++ eve
        ++ "\ncloudiness: "
        ++ clouds
        ++ "%\nprobability of precip: "
        ++ pop
        ++ "%",
      );
    | Error(MissingEnv(env)) => Js.Console.error("Missing Env Va: " ++ env)
    | Error(App.TooCloudy(pct)) =>
      Js.Console.error("Too Cloudy -- " ++ Js.Float.toString(pct) ++ "%")
    | Error(App.TooRainy(pop)) =>
      Js.Console.error("Too Rainy -- " ++ Js.Float.toString(pop) ++ "%")
    | Error(App.TooCold(tmp)) =>
      Js.Console.error("Too Rainy -- " ++ Js.Float.toString(tmp))
    | Error(AWS.Error(err)) => Js.Console.error(err)
    | Error(err) => Js.Console.error(err)
    }
  );
